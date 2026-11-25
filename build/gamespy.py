#!/usr/bin/env python3
"""GameSpy's query-and-reporting 2 (qr2) and server browsing (sb, list
protocol 1, encoding 3) as Tron 2.0's SDK speaks them (src/Game/Libs/
GameSpy/GameSpySDK/qr2/qr2.c, serverbrowsing/sb_serverlist.c,
sb_crypt.c). Written for the modern build's tests (docs/MODERN.md GS-1,
GS-2); it has no code of GameSpy's or OpenSpy's.

  gamespy.py master [--bind 127.0.0.1] [--log FILE] [--secret KEY]
      A local stand-in for the master server: UDP 27900 takes the servers'
      heartbeats, challenges each server and lists it when its answer is
      correct; TCP 28910 gives the list to a browser, encrypted as the SDK
      expects. Each event is a line on stdout (and in FILE).

  gamespy.py list HOST [--game tron20] [--secret KEY] [--fields ...]
      A browser: asks HOST (TCP 28910) for the list of a game and prints
      each server. With --query, also asks each server for its keys (qr2,
      UDP). Read only: it sends a list request and nothing else.

  gamespy.py host GAME [--domain openspy.net]
      The master server list host of GAME, "<game>.ms<n>.<domain>".
"""
import argparse
import os
import random
import select
import socket
import struct
import sys
import time

MASTER_PORT = 27900   # qr2.c MASTER_PORT (UDP)
MSPORT2 = 28910       # sb_internal.h MSPORT2 (TCP)
NUM_MASTER_SERVERS = 20
QR_MAGIC = b"\xfe\xfd"
# qr2.c packet types
P_QUERY, P_CHALLENGE, P_ECHO, P_HEARTBEAT, P_ADDERROR, P_ECHO_RESPONSE, \
    P_CLIENT_MESSAGE, P_CLIENT_MESSAGE_ACK, P_KEEPALIVE = 0, 1, 2, 3, 4, 5, 6, 7, 8
# sb_internal.h server flags
UNSOLICITED_UDP_FLAG, PRIVATE_IP_FLAG, CONNECT_NEGOTIATE_FLAG, ICMP_IP_FLAG, \
    NONSTANDARD_PORT_FLAG, NONSTANDARD_PRIVATE_PORT_FLAG, HAS_KEYS_FLAG, \
    HAS_FULL_RULES_FLAG = 1, 2, 4, 8, 16, 32, 64, 128
TRON_GAME = "tron20"
TRON_SECRET = "t9D3vB"   # TronGameServerShell.cpp, BaseScreenJoin.cpp

LOGFILE = None


def log(msg):
    line = "%s %s" % (time.strftime("%H:%M:%S"), msg)
    print(line, flush=True)
    if LOGFILE:
        with open(LOGFILE, "a") as f:
            f.write(line + "\n")


# --- sb_serverlist.c StringHash: which of the 20 list hosts --------------

def string_hash(s, buckets=NUM_MASTER_SERVERS):
    h = 0
    for c in s:
        h = (h * -1664117991 + ord(c.lower())) & 0xFFFFFFFF
    return h % buckets


# --- qr2.c: the server's answer to the master's challenge ----------------

def gs_encrypt(key, data):
    state = list(range(256))
    x = y = 0
    for i in range(256):
        y = (key[x] + state[i] + y) % 256
        x = (x + 1) % len(key)
        state[i], state[y] = state[y], state[i]
    out = bytearray(data)
    x = y = 0
    for i in range(len(out)):
        x = (x + out[i] + 1) % 256
        y = (state[x] + y) % 256
        state[x], state[y] = state[y], state[x]
        out[i] ^= state[(state[x] + state[y]) % 256]
    return bytes(out)


def gs_encode(data):
    ct = b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
    out = bytearray()
    for i in range(0, len(data), 3):
        t = data[i:i + 3] + b"\0" * (3 - len(data[i:i + 3]))
        out += bytes([ct[t[0] >> 2], ct[((t[0] & 3) << 4) + (t[1] >> 4)],
                      ct[((t[1] & 15) << 2) + (t[2] >> 6)], ct[t[2] & 63]])
    return bytes(out)


def challenge_response(secret, challenge):
    return gs_encode(gs_encrypt(secret.encode(), challenge))


# --- sb_crypt.c: the list's cipher ----------------------------------------

class GOACrypt:
    def __init__(self, key):
        self.cards = list(range(256))
        rsum = 0
        keypos = 0

        def keyrand(limit):
            nonlocal rsum, keypos
            if not limit:
                return 0
            retry = 0
            mask = 1
            while mask < limit:
                mask = (mask << 1) + 1
            while True:
                rsum = (self.cards[rsum] + key[keypos]) & 0xFF
                keypos += 1
                if keypos >= len(key):
                    keypos = 0
                    rsum = (rsum + len(key)) & 0xFF
                u = mask & rsum
                retry += 1
                if retry > 11:
                    u %= limit
                if u <= limit:
                    return u
        for i in range(255, -1, -1):
            t = keyrand(i)
            self.cards[i], self.cards[t] = self.cards[t], self.cards[i]
        c = self.cards
        self.rotor, self.ratchet, self.avalanche = c[1], c[3], c[5]
        self.last_plain, self.last_cipher = c[7], c[rsum]

    def _shuffle(self):
        c = self.cards
        self.ratchet = (self.ratchet + c[self.rotor]) & 0xFF
        self.rotor = (self.rotor + 1) & 0xFF
        s = c[self.last_cipher]
        c[self.last_cipher] = c[self.ratchet]
        c[self.ratchet] = c[self.last_plain]
        c[self.last_plain] = c[self.rotor]
        c[self.rotor] = s
        self.avalanche = (self.avalanche + c[s]) & 0xFF

    def _mask(self):
        c = self.cards
        return c[(c[self.avalanche] + c[self.rotor]) & 0xFF] ^ \
            c[c[(c[self.last_plain] + c[self.last_cipher] + c[self.ratchet]) & 0xFF]]

    def encrypt(self, data):
        out = bytearray()
        for b in data:
            self._shuffle()
            self.last_cipher = b ^ self._mask()
            self.last_plain = b
            out.append(self.last_cipher)
        return bytes(out)

    def decrypt(self, data):
        out = bytearray()
        for b in data:
            self._shuffle()
            self.last_plain = b ^ self._mask()
            self.last_cipher = b
            out.append(self.last_plain)
        return bytes(out)


def list_crypt_key(secret, client_challenge, server_challenge):
    """sb_serverlist.c InitCryptKey."""
    ch = bytearray(client_challenge)
    sk = secret.encode()
    for i in range(len(server_challenge)):
        ch[(i * sk[i % len(sk)]) % 8] ^= (ch[i % 8] ^ server_challenge[i]) & 0xFF
    return GOACrypt(bytes(ch))


def nts(buf, pos):
    end = buf.index(b"\0", pos)
    return buf[pos:end].decode("latin-1"), end + 1


# --- the master server ----------------------------------------------------

class Master:
    def __init__(self, bind, secret):
        self.secret = secret
        self.servers = {}   # (ip, port) -> dict
        self.udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.udp.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.udp.bind((bind, MASTER_PORT))
        self.tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.tcp.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.tcp.bind((bind, MSPORT2))
        self.tcp.listen(8)
        log("master: UDP %s:%d, TCP %s:%d" % (bind, MASTER_PORT, bind, MSPORT2))

    def run(self):
        while True:
            r, _, _ = select.select([self.udp, self.tcp], [], [], 1.0)
            if self.udp in r:
                data, addr = self.udp.recvfrom(4096)
                self.on_udp(data, addr)
            if self.tcp in r:
                conn, addr = self.tcp.accept()
                try:
                    self.on_list(conn, addr)
                except Exception as e:  # keep serving
                    log("list: %s:%d: error %r" % (addr[0], addr[1], e))
                finally:
                    conn.close()
            now = time.time()
            for k in [k for k, s in self.servers.items() if now - s["seen"] > 300]:
                log("server %s:%d: gone (no heartbeat for 300 s)" % k)
                del self.servers[k]

    def on_udp(self, data, addr):
        if len(data) < 5:
            return
        ptype, ikey, body = data[0], data[1:5], data[5:]
        srv = self.servers.get(addr)
        if ptype == P_HEARTBEAT:
            keys = {}
            parts = body.split(b"\0")
            i = 0
            while i + 1 < len(parts) and parts[i]:
                keys[parts[i].decode("latin-1")] = parts[i + 1].decode("latin-1")
                i += 2
            if keys.get("statechanged") == "2":
                if addr in self.servers:
                    del self.servers[addr]
                log("server %s:%d: stopped (statechanged 2)" % addr)
                return
            if srv is None:
                srv = self.servers[addr] = {"listed": False}
            srv.update(seen=time.time(), keys=keys, ikey=ikey)
            log("heartbeat %s:%d: %s" % (addr + (" ".join(
                "%s=%s" % (k, keys[k]) for k in ("gamename", "hostname", "mapname",
                "gametype", "numplayers", "maxplayers", "hostport", "localport",
                "statechanged") if k in keys),)))
            if keys.get("gamename") != TRON_GAME:
                log("server %s:%d: game %r is not %s" % (addr + (keys.get("gamename"), TRON_GAME)))
                return
            if not srv["listed"] or keys.get("statechanged") == "3":
                chal = "".join(random.choice("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") for _ in range(8))
                srv["challenge"] = chal
                self.udp.sendto(QR_MAGIC + bytes([P_CHALLENGE]) + ikey + chal.encode() + b"\0", addr)
                log("challenge %s:%d: %s" % (addr + (chal,)))
        elif ptype == P_CHALLENGE and srv is not None:
            got = body.split(b"\0")[0]
            want = challenge_response(self.secret, srv.get("challenge", "").encode())
            if got == want:
                srv["listed"] = True
                log("server %s:%d: listed (the challenge answer is correct: %s)" % (addr + (got.decode(),)))
            else:
                log("server %s:%d: wrong challenge answer %r, want %r" % (addr + (got, want)))
        elif ptype == P_KEEPALIVE:
            if srv is not None:
                srv["seen"] = time.time()
        else:
            log("udp %s:%d: packet type %d (%d bytes)" % (addr + (ptype, len(data))))

    def on_list(self, conn, addr):
        conn.settimeout(10)
        buf = b""
        while len(buf) < 2 or len(buf) < struct.unpack(">H", buf[:2])[0]:
            d = conn.recv(4096)
            if not d:
                return
            buf += d
        reqlen = struct.unpack(">H", buf[:2])[0]
        req = buf[:reqlen]
        if req[2] != 0:
            log("list %s:%d: request type %d, not a list" % (addr + (req[2],)))
            return
        pver, enc = req[3], req[4]
        fromver = struct.unpack("<I", req[5:9])[0]
        forgame, p = nts(req, 9)
        fromgame, p = nts(req, p)
        cchal = req[p:p + 8]; p += 8
        filt, p = nts(req, p)
        fields, p = nts(req, p)
        opts = struct.unpack(">I", req[p:p + 4])[0]
        log("list %s:%d: protocol %d encoding %d game %s from %s fields %r filter %r options %d"
            % (addr + (pver, enc, forgame, fromgame, fields, filt, opts)))
        schal = bytes(random.randrange(33, 126) for _ in range(10))
        rnd = bytes(random.randrange(256) for _ in range(4))
        head = bytes([len(rnd) ^ 0xEC]) + rnd + bytes([len(schal) ^ 0xEA]) + schal
        crypt = list_crypt_key(self.secret, cchal, schal)
        ip = socket.inet_aton(addr[0])
        if forgame != TRON_GAME:
            body = ip + b"\xff\xff" + ("unknown game %s" % forgame).encode() + b"\0"
            conn.sendall(head + crypt.encrypt(body))
            return
        body = ip + struct.pack(">H", 0)   # no standard port: each server has its own
        listed = [(k, s) for k, s in self.servers.items() if s["listed"]]
        if not (opts & 2):   # NO_SERVER_LIST
            body += b"\0"    # no keys: the browser asks each server itself
            body += b"\0"    # no popular values
            for (sip, sport), s in listed:
                body += bytes([NONSTANDARD_PORT_FLAG]) + socket.inet_aton(sip) + struct.pack(">H", sport)
            body += b"\0\xff\xff\xff\xff"
        conn.sendall(head + crypt.encrypt(body))
        log("list %s:%d: sent %d server(s): %s" % (addr + (len(listed),
            ", ".join("%s:%d %r" % (k[0], k[1], s["keys"].get("hostname")) for k, s in listed))))
        time.sleep(0.5)


# --- a browser ------------------------------------------------------------

def get_list(host, game, secret, fields, timeout=10):
    sock = socket.create_connection((host, MSPORT2), timeout=timeout)
    cchal = bytearray()
    for _ in range(8):
        cchal.append(random.randrange(33, 126))
    body = bytes([0, 1, 3]) + struct.pack("<I", 0) + game.encode() + b"\0" + \
        game.encode() + b"\0" + bytes(cchal) + b"\0" + fields.encode() + b"\0" + \
        struct.pack(">I", 0)
    sock.sendall(struct.pack(">H", len(body) + 2) + body)
    data = b""
    end = time.time() + timeout
    while time.time() < end:
        try:
            d = sock.recv(65536)
        except socket.timeout:
            break
        if not d:
            break
        data += d
        if len(data) > 1:
            n = (data[0] ^ 0xEC) + 2
            if len(data) > n:
                m = n + (data[n - 1] ^ 0xEA)
                if len(data) >= m:
                    crypt = list_crypt_key(secret, bytes(cchal), data[n:m])
                    plain = crypt.decrypt(data[m:])
                    if plain.endswith(b"\0\xff\xff\xff\xff") or \
                            (len(plain) >= 6 and plain[4:6] == b"\xff\xff" and b"\0" in plain[6:]):
                        break
    sock.close()
    n = (data[0] ^ 0xEC) + 2 if data else 0
    if not data or len(data) <= n or len(data) < n + (data[n - 1] ^ 0xEA) + 6:
        raise SystemExit("%s gave no list for %s (%d bytes)" % (host, game, len(data)))
    m = n + (data[n - 1] ^ 0xEA)
    plain = list_crypt_key(secret, bytes(cchal), data[n:m]).decrypt(data[m:])
    myip = socket.inet_ntoa(plain[0:4])
    port = struct.unpack(">H", plain[4:6])[0]
    if port == 0xFFFF:
        err = plain[6:].split(b"\0")[0].decode("latin-1")
        return myip, None, err
    if not plain.endswith(b"\0\xff\xff\xff\xff"):
        raise SystemExit("%s: the list does not decrypt (a wrong secret key?)" % host)
    p = 6
    nkeys = plain[p]; p += 1
    keys = []
    for _ in range(nkeys):
        kt = plain[p]
        name, p = nts(plain, p + 1)
        keys.append((kt, name))
    npop = plain[p]; p += 1
    pop = []
    for _ in range(npop):
        v, p = nts(plain, p)
        pop.append(v)
    servers = []
    while p + 5 <= len(plain):
        flags = plain[p]
        if plain[p + 1:p + 5] == b"\xff\xff\xff\xff":
            break
        ip = socket.inet_ntoa(plain[p + 1:p + 5]); p += 5
        sport = port
        if flags & NONSTANDARD_PORT_FLAG:
            sport = struct.unpack(">H", plain[p:p + 2])[0]; p += 2
        if flags & PRIVATE_IP_FLAG:
            p += 4
        if flags & ICMP_IP_FLAG:
            p += 4
        if flags & NONSTANDARD_PRIVATE_PORT_FLAG:
            p += 2
        vals = {}
        if flags & HAS_KEYS_FLAG:
            for kt, name in keys:
                if kt == 1:
                    vals[name] = plain[p]; p += 1
                elif kt == 2:
                    vals[name] = struct.unpack(">H", plain[p:p + 2])[0]; p += 2
                else:
                    idx = plain[p]; p += 1
                    if idx == 0xFF:
                        vals[name], p = nts(plain, p)
                    else:
                        vals[name] = pop[idx]
        if flags & HAS_FULL_RULES_FLAG:
            while plain[p] != 0:
                k, p = nts(plain, p)
                v, p = nts(plain, p)
                vals[k] = v
            p += 1
        servers.append((ip, sport, flags, vals))
    return myip, servers, None


def query_server(ip, port, timeout=3):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.settimeout(timeout)
    s.sendto(QR_MAGIC + bytes([P_QUERY]) + b"TRON" + b"\xff\x00\x00", (ip, port))
    try:
        data, _ = s.recvfrom(4096)
    except socket.timeout:
        return None
    parts = data[5:].split(b"\0")
    vals = {}
    i = 0
    while i + 1 < len(parts) and parts[i]:
        vals[parts[i].decode("latin-1")] = parts[i + 1].decode("latin-1")
        i += 2
    return vals


def main():
    global LOGFILE
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    m = sub.add_parser("master")
    m.add_argument("--bind", default="127.0.0.1")
    m.add_argument("--log")
    m.add_argument("--secret", default=TRON_SECRET)
    l = sub.add_parser("list")
    l.add_argument("host")
    l.add_argument("--game", default=TRON_GAME)
    l.add_argument("--secret", default=TRON_SECRET)
    l.add_argument("--fields", default="\\hostname\\gamever\\mapname\\gametype\\numplayers\\maxplayers\\hostport")
    l.add_argument("--query", action="store_true")
    h = sub.add_parser("host")
    h.add_argument("game")
    h.add_argument("--domain", default="openspy.net")
    a = ap.parse_args()
    if a.cmd == "master":
        LOGFILE = a.log
        Master(a.bind, a.secret).run()
    elif a.cmd == "host":
        print("%s.ms%d.%s" % (a.game, string_hash(a.game), a.domain))
    else:
        myip, servers, err = get_list(a.host, a.game, a.secret, a.fields)
        if err is not None:
            print("error from the master: %s" % err)
            return 1
        print("%d server(s) for %s" % (len(servers), a.game))
        for ip, port, flags, vals in servers:
            line = "%s:%d flags 0x%02x %s" % (ip, port, flags, " ".join("%s=%s" % kv for kv in vals.items()))
            if a.query:
                q = query_server(ip, port)
                line += " | query: %s" % (q if q is not None else "no answer")
            print(line)
    return 0


if __name__ == "__main__":
    sys.exit(main())
