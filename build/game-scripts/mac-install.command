#!/usr/bin/perl
# mac-install.command -- install the game data from your Tron 2.0 discs, on
# a Mac (to play the game with CrossOver, or another Wine).
#
#   Double-click it in Finder with the disc images in this folder (or the
#   one above it), or in Terminal:
#   ./mac-install.command <disc image> <disc image>
#
# The images are the two discs of Tron 2.0 (USA), in either order, each as
# any of
#
#   .iso   a plain image (2048-byte sectors), e.g. your own rip
#   .bin   a raw image (2352-byte sectors, e.g. redump's), or its .cue
#   .zip   a .zip that holds one such .bin (and its .cue)
#
# No update is needed: the data of Update 2 (v1.030) is in this release
# (tron20src.rez), and its programs are built from source.
#
# What linux-install.sh does: first the script checks that this folder is
# the release unpacked, and nothing else (every file SHA256SUMS lists is
# here and unchanged, and no file is here that the release does not have,
# nor one the game writes as it runs: it refuses a release copied over
# another installation). A .bin, .cue or .zip is then converted to a
# temporary .iso in this folder (about 0.7 GB a disc, removed at the end).
# Then it checks each file's SHA-256, copies the game's data out of them
# into this folder -- the .rez archives (with the retail game DLLs inside
# them zeroed: the game DLLs here are built from source), the movies, the
# manual, the shipped settings, and Bink and the
# Microsoft runtime where this build uses them -- and checks every file it
# writes. Nothing is installed or run from the discs, and
# nothing else is changed. It can be run again: a file already in place is
# checked and kept.
#
# Tron 2.0's setup stores its files compressed, in InstallShield cabinets.
# This needs only the Perl every Mac has (/usr/bin/perl, with its own
# modules: Compress::Raw::Zlib inflates). The list of files, after this
# script's __END__, says for each one which bytes of which image it is made
# of (build/mkinstall.py).
#
# Then play with CrossOver (or another Wine): the modern build's Tron.exe
# (or TronLauncher.exe, the launcher), which needs nothing from the
# registry. The legacy build's Tron.exe is the retail launcher, which wants
# the retail setup's registry values and disc 1 in a drive (linux-run.sh
# shows what it needs); without them, run Lithtech.exe -cmdfile
# tron20cmds.txt.
use strict;
use warnings;
use Cwd qw(abs_path);
use File::Basename qw(basename dirname);
use File::Find qw(find);
use File::Path qw(make_path);
use Digest::SHA;
use MIME::Base64 qw(decode_base64);
use Compress::Raw::Zlib;

$| = 1;
my $ME = 'mac-install';
my $HERE = dirname(abs_path($0));
my $CHUNK = 1 << 20;
my @TEMPS;    # converted images, removed however the script ends

sub cleanup { unlink grep { -e } @TEMPS; @TEMPS = (); }
END { cleanup(); }
$SIG{INT} = $SIG{TERM} = $SIG{HUP} = sub { cleanup(); exit 1; };
sub fail { print STDERR "\n$ME: $_[0]\n"; cleanup(); exit 1; }

sub sha256_file {
    my $d = Digest::SHA->new(256);
    $d->addfile($_[0], 'b') or fail("$_[0]: cannot read it");
    return $d->hexdigest;
}

# The list: every line after __END__ and its marker line.
my @data;
{
    my $m = <DATA>;
    defined $m && $m =~ /^__\w+_INSTALL_DATA__\r?\n\z/
        or fail('the file list is missing from the script');
    while (my $l = <DATA>) {
        $l =~ s/\r?\n\z//;
        push @data, $l if $l ne '';
    }
    @data && $data[0] =~ /^\w+INSTALL 1\z/
        or fail('the file list is not one this script reads');
}

# This folder: the release unpacked, and nothing else? SHA256SUMS (the
# archive's last file) lists every file the archive holds and every file
# this script writes. Each of the archive's must be here as it was shipped,
# and nothing may be here that is neither, nor what the game writes as it
# runs (saves, profiles, server configurations, settings, logs,
# screenshots, the launcher's command file, mods) nor a disc image --
# which is what copying the release over another installation,
# or over an older release, leaves. (linux-install.sh's check_dir.)
sub check_dir {
    my $sums = "$HERE/SHA256SUMS";
    -f $sums or fail('SHA256SUMS is missing: run the script in the folder the release unpacked to');
    my (%SUM, %WRITE, @bad, @extra);
    open my $fh, '<', $sums or fail("$sums: cannot read it");
    while (my $l = <$fh>) {
        $l =~ s/\r?\n\z//;
        $SUM{$2} = $1 if $l =~ /^(\S+)  (.*)\z/;
    }
    close $fh;
    for (@data) { $WRITE{(split / /, $_, 6)[5]} = 1 if /^F /; }
    print "checking this folder ...\n";
    for my $rel (sort keys %SUM) {
        next if $WRITE{$rel};                        # this script writes it
        if (!-f "$HERE/$rel") { push @bad, "missing: $rel"; }
        elsif (sha256_file("$HERE/$rel") ne $SUM{$rel}) { push @bad, "changed: $rel"; }
    }
    my @here;
    find({ no_chdir => 1, wanted => sub {
        push @here, substr($File::Find::name, length($HERE) + 1) if -f $_ || -l $_;
    } }, $HERE);
    for my $rel (sort @here) {
        next if exists $SUM{$rel} || $WRITE{$rel};
        my $l = lc $rel;
        next if $l =~ m{\A(?:sha256sums|autoexec\.cfg|display\.cfg|tron20\.ini|launchcmds\.txt|run-cmds\.txt|banlist\.txt)\z};
        next if $l =~ m{\A(?:save|profiles|serverdata|custom/mods|custom/resources)/};
        next if $l =~ m{(?:\A|/)(?:thumbs\.db|desktop\.ini|\.ds_store)\z} || $l =~ /\.part\z/;
        next if $l !~ m{/} && $l =~ /\.(?:log|bmp|tga|iso|bin|cue|zip)\z/;   # top level only
        push @extra, $rel;
    }
    return if !@bad && !@extra;
    print STDERR "\n$ME: this folder is not just this release, unpacked:\n";
    print STDERR "  $_\n" for @bad;
    print STDERR "  not part of it: $_\n" for @extra[0 .. ($#extra < 19 ? $#extra : 19)];
    print STDERR "  ... and ", @extra - 20, " more files not part of it\n" if @extra > 20;
    print STDERR "\nThe release is the whole game, not a patch: do not copy it over another\n",
                 "installation, or over an older release. Unpack it into a new, empty\n",
                 "folder and run mac-install.command there. (Your saved games and profiles\n",
                 "are in the old folder's Save/ and Profiles/: copy those folders across\n",
                 "afterwards.)\n";
    exit 1;
}
check_dir();

# The images: their sizes and checksums, and the sets the game installs
# from (a name, then its images).
my (%SIZE, %SHA, %NAME, %RAW, %HAVE, @SETS);
for (@data) {
    if (/^D (\S+) (\d+) (\S+) (.*)\z/) { ($SIZE{$1}, $SHA{$1}, $NAME{$1}) = ($2, $3, $4); }
    elsif (/^R (\S+) (\d+)\z/) { $RAW{$1} = $2; }
    elsif (/^S (\S+) (.*)\z/) { push @SETS, [$1, split / /, $2]; }
}

# Is there a set every image of which is found?
sub complete {
    SET: for my $s (@SETS) {
        my (undef, @ids) = @$s;
        $HAVE{$_} or next SET for @ids;
        return 1;
    }
    return 0;
}

# A .zip's members from its central directory: (name, method, packed size,
# size, offset of the local header) of each. Not zip64: redump's discs fit.
sub zip_members {
    my ($path) = @_;
    open my $fh, '<:raw', $path or return ();
    my $len = -s $fh;
    my $tail = $len < 65557 ? $len : 65557;
    sysseek($fh, $len - $tail, 0);
    sysread($fh, my $buf, $tail);
    my $at = rindex($buf, "PK\x05\x06");
    return () if $at < 0 || length($buf) - $at < 22;
    my ($count, $cdsize, $cdoff) = unpack('x10 v V V', substr($buf, $at, 22));
    sysseek($fh, $cdoff, 0);
    (sysread($fh, my $cd, $cdsize) // -1) == $cdsize or return ();
    my (@m, $p);
    $p = 0;
    for (1 .. $count) {
        last if substr($cd, $p, 4) ne "PK\x01\x02";
        my ($method, $packed, $size, $nlen, $xlen, $clen, $local) =
            unpack('x10 v x8 V V v v v x8 V', substr($cd, $p, 46));
        push @m, [substr($cd, $p + 46, $nlen), $method, $packed, $size, $local];
        $p += 46 + $nlen + $xlen + $clen;
    }
    close $fh;
    return @m;
}

# The one .bin of a .zip, or undef (then the .zip is not a disc).
sub zip_bin {
    my @bins = grep { $_->[0] =~ /\.bin\z/i } zip_members($_[0]);
    return @bins == 1 ? $bins[0] : undef;
}

# A redump image's raw (2352-byte sector) size: a .bin's, or a .zip's .bin's.
sub raw_size {
    my ($f) = @_;
    if ($f =~ /\.zip\z/i) { my $b = zip_bin($f); return $b ? $b->[3] : 0; }
    return -s $f;
}

# A reader of a redump image's raw bytes: a code ref giving up to n bytes
# (fewer only at the end).
sub raw_reader {
    my ($f) = @_;
    if ($f !~ /\.zip\z/i) {
        open my $fh, '<:raw', $f or fail("$f: cannot read it");
        return sub { my $n = sysread($fh, my $b, $_[0]); defined $n or fail("$f: a read error"); $b };
    }
    my $m = zip_bin($f) or fail("$f: not one .bin in it");
    return zip_reader($f, $m);
}

# A reader of one member of a .zip (zip_members' row), as raw_reader's.
sub zip_reader {
    my ($f, $m) = @_;
    open my $fh, '<:raw', $f or fail("$f: cannot read it");
    my ($name, $method, $packed, undef, $local) = @$m;
    sysseek($fh, $local, 0);
    my $h;
    (sysread($fh, $h, 30) // 0) == 30 && substr($h, 0, 4) eq "PK\x03\x04" or fail("$f: a damaged .zip");
    my ($nlen, $xlen) = unpack('x26 v v', $h);
    sysseek($fh, $local + 30 + $nlen + $xlen, 0);
    my $left = $packed;
    my $inf;
    if ($method == 8) {
        ($inf) = Compress::Raw::Zlib::Inflate->new(-WindowBits => -MAX_WBITS, -Bufsize => 1 << 20);
        $inf or fail("$f: cannot inflate it");
    } elsif ($method != 0) {
        fail("$f: $name is compressed in a way this script cannot read (method $method)");
    }
    my $out = '';
    return sub {
        my ($want) = @_;
        while (length($out) < $want && $left > 0) {
            my $n = sysread($fh, my $b, $left < $CHUNK ? $left : $CHUNK);
            defined $n && $n > 0 or fail("$f: the .zip ended early");
            $left -= $n;
            if ($inf) {
                my $st = $inf->inflate($b, my $o);
                $st == Z_OK || $st == Z_STREAM_END or fail("$f: a damaged .zip ($st)");
                $out .= $o;
            } else { $out .= $b; }
        }
        return substr($out, 0, $want, '');
    };
}

# The .iso of a redump image: the 2048 bytes of user data of each sector,
# after the 16-byte sync and header (MODE1, the European discs) and the
# 8-byte subheader of MODE2 (the USA ones), the track's mode as its first
# sector's header gives it.
sub convert {
    my ($f, $to) = @_;
    my $read = raw_reader($f);
    open my $out, '>:raw', $to or fail("$to: cannot write it");
    my ($S, $k) = (2352, undef);
    while (1) {
        my $buf = $read->($S * 512);
        last if $buf eq '';
        length($buf) % $S == 0 or fail(basename($f) . ': not whole 2352-byte sectors');
        $k //= ord(substr($buf, 15, 1)) == 2 ? 24 : 16;
        my $o = '';
        for (my $i = 0; $i < length($buf); $i += $S) { $o .= substr($buf, $i + $k, 2048); }
        print $out $o or fail("$to: cannot write it (is the disk full?)");
    }
    close $out or fail("$to: cannot write it (is the disk full?)");
}

# take(file, label): which image a file is, if any (by size, then SHA-256).
sub take {
    my ($f, $label) = @_;
    my $size = -s $f;
    my $sum;
    for my $id (sort keys %SIZE) {
        next if $HAVE{$id} || $size != $SIZE{$id};
        if (!defined $sum) {
            print "checking $label ...\n";
            $sum = sha256_file($f);
        }
        if ($sum eq $SHA{$id}) {
            $HAVE{$id} = $f;
            print "  $NAME{$id}\n";
            return 1;
        }
    }
    print "  the size of an image, but not its contents (a bad copy?)\n" if defined $sum;
    return 0;
}

my @candidates = @ARGV;
if (!@candidates) {
    # .iso first: one is used as it is, the others need converting.
    for my $ext ('iso', 'bin', 'zip') {
        for my $dir ($HERE, dirname($HERE)) {
            opendir(my $dh, $dir) or next;
            push @candidates, map { "$dir/$_" } sort grep { /\.\Q$ext\E\z/i && -f "$dir/$_" } readdir $dh;
            closedir $dh;
        }
    }
}
for my $f (@candidates) {
    last if complete();
    -f $f or fail("$f: no such file");
    my $label = basename($f);
    if ($f =~ /\.cue\z/i) {
        open my $fh, '<', $f or fail("$f: cannot read it");
        my $cue = do { local $/; <$fh> };
        close $fh;
        my ($bin) = $cue =~ /^\s*FILE\s*"([^"]+)"/m;
        if (!defined $bin || $cue !~ /TRACK\s*01\s*MODE[12]\/2352/i) {
            print "  $label: not a one-track MODE1/2352 or MODE2/2352 cue sheet; skipped\n";
            next;
        }
        $f = dirname($f) . "/$bin";
        -f $f or fail("$label names $bin, which is not beside it");
    }
    if ($f =~ /\.(cue|bin|zip)\z/i) {
        my $size = raw_size($f);
        for my $id (sort keys %RAW) {
            next if $HAVE{$id} || $size != $RAW{$id};
            my $tmp = "$HERE/.disc-" . scalar(@TEMPS) . ".iso.part";
            push @TEMPS, $tmp;
            print "converting $label to an .iso ...\n";
            convert($f, $tmp);
            take($tmp, "$label (converted)") or unlink $tmp;
            last;
        }
    } else {
        take($f, $label);
    }
}
if (!complete()) {
    print STDERR "$ME: these are needed, and not all were found:\n";
    for my $s (@SETS) {
        my ($name, @ids) = @$s;
        print STDERR "  $name:\n";
        for my $id (@ids) {
            if ($HAVE{$id}) { print STDERR "    found:   $NAME{$id}\n"; }
            else { print STDERR "    missing: $NAME{$id} ($SIZE{$id} bytes, SHA-256 $SHA{$id})\n"; }
        }
    }
    print STDERR "  (a disc as an .iso, or redump's .bin/.cue or .zip of it)\n",
                 "\nPut them in this folder and run it again, or give them to it in Terminal:\n",
                 "./mac-install.command disc1.iso disc2.iso\n";
    exit 1;
}

# A raw deflate stream that goes on from `history` (the output before it):
# inflated behind a stored block of that history, which its back-references
# then reach -- what zlib's preset dictionary does, with any zlib.
sub inflate_raw {
    my ($history, $in) = @_;
    my $pre = '';
    if (length $history) {
        $pre = "\x00" . pack('vv', length($history), ~length($history) & 0xffff) . $history;
    }
    my ($inf) = Compress::Raw::Zlib::Inflate->new(-WindowBits => -MAX_WBITS, -Bufsize => 1 << 16);
    $inf or fail('cannot inflate');
    my $stream = $pre . $in;
    my $st = $inf->inflate($stream, my $out);
    $st == Z_STREAM_END or fail("a compressed block does not inflate ($st)");
    return substr($out, length $history);
}

# The ways to read each retail file: the first whose images are all here.
my (%ways, $vid);
for (@data) {
    my @p = split / /;
    if ($p[0] eq 'V') { $vid = $p[1]; }
    elsif ($p[0] eq 'W' && !$ways{$vid}) {
        my @r = map { [split /:/] } @p[2 .. $#p];
        next if grep { !$HAVE{$_->[0]} } @r;
        $ways{$vid} = [$p[1], \@r];
    }
}

# A retail file's bytes, in order: a code ref giving the next piece, or
# undef at the end.
my %streams;
sub ranges_reader {
    my @r = map { [@$_] } @{$_[0]};
    return sub {
        while (@r) {
            my $x = $r[0];
            my ($id, $off, $n) = @$x;
            if ($n == 0) { shift @r; next; }
            my $fh = $streams{$id};
            if (!$fh) { open($fh, '<:raw', $HAVE{$id}) or fail("$HAVE{$id}: cannot read it"); $streams{$id} = $fh; }
            sysseek($fh, $off, 0) or fail("$HAVE{$id}: cannot seek");
            my $got = sysread($fh, my $b, $n < $CHUNK ? $n : $CHUNK);
            defined $got && $got > 0 or fail("$HAVE{$id} ended early");
            $x->[1] += $got;
            $x->[2] -= $got;
            return $b;
        }
        return undef;
    };
}

# InstallShield 6: chunks of a 16-bit length and a raw deflate stream.
sub inflate_reader {
    my $src = ranges_reader($_[0]);
    my $buf = '';
    my $need = sub {
        while (length($buf) < $_[0]) {
            my $m = $src->();
            return 0 unless defined $m;
            $buf .= $m;
        }
        return 1;
    };
    return sub {
        return undef unless $need->(2);
        my $n = unpack('v', $buf);
        $need->(2 + $n) or fail('a compressed file ends early');
        my $out = inflate_raw('', substr($buf, 2, $n));
        substr($buf, 0, 2 + $n, '');
        return $out;
    };
}

# The files.
sub ok {   # ok(path, size, sha): is the file there, and right?
    my ($p, $size, $sha) = @_;
    return -f $p && -s $p == $size && sha256_file($p) eq $sha;
}
my ($written, $kept, $skip) = (0, 0, 1);
my ($out, $name, $dest, $part, $size, $sha, $src, $sbuf);
# take n bytes of the retail file, writing them out unless zeros or ours
# are written in their place
sub consume {
    my ($n, $write) = @_;
    while ($n > 0) {
        while ($sbuf eq '') {
            $sbuf = $src->();
            defined $sbuf or fail('a retail file ends early');
        }
        my $k = length($sbuf) < $n ? length($sbuf) : $n;
        if ($write) { print $out substr($sbuf, 0, $k) or fail("$part: cannot write it (is the disk full?)"); }
        substr($sbuf, 0, $k, '');
        $n -= $k;
    }
}
for (@data) {
    my @p = split / /;
    my $tag = $p[0];
    if ($tag eq 'F') {
        my ($flag, $v);
        (undef, $size, $sha, $flag, $v, $name) = split / /, $_, 6;   # (the name may have spaces)
        $dest = "$HERE/$name";
        $part = "$dest.part";
        $skip = ($flag eq 'once' && -e $dest) || ok($dest, $size, $sha);
        if ($skip) { $kept++; next; }
        print "$name\n";
        $v eq '-' || $ways{$v} or fail("$name: none of the images found has it");
        make_path(dirname($dest));
        open($out, '>:raw', $part) or fail("$part: cannot write it");
        # (source "-": a file of ours, only L records)
        $src = undef;
        if ($v ne '-') {
            my ($kind, $ranges) = @{$ways{$v}};
            $src = $kind eq 'copy' ? ranges_reader($ranges) : inflate_reader($ranges);
        }
        $sbuf = '';
    } elsif ($skip || $tag !~ /^[CZLE]\z/) {
        next;
    } elsif ($tag eq 'C') {
        consume($p[1], 1);
    } elsif ($tag eq 'Z') {
        consume($p[1], 0);
        my $left = $p[1];
        while ($left > 0) {
            my $n = $left < $CHUNK ? $left : $CHUNK;
            print $out "\0" x $n or fail("$part: cannot write it (is the disk full?)");
            $left -= $n;
        }
    } elsif ($tag eq 'L') {
        my $b = decode_base64($p[1]);
        consume(length $b, 0) if $src;
        print $out $b or fail("$part: cannot write it (is the disk full?)");
    } elsif ($tag eq 'E') {
        close $out or fail("$part: cannot write it (is the disk full?)");
        if (!ok($part, $size, $sha)) {
            unlink $part;
            fail("$name came out wrong (a disk error?); the files before it are in place");
        }
        rename($part, $dest) or fail("$dest: cannot move it into place");
        $written++;
    }
}
close $_ for values %streams;
make_path("$HERE/Save");
print "\ndone: $written files written, $kept already in place.\n";
if (-f "$HERE/TronLauncher.exe") {
    print "Play by running Tron.exe in CrossOver (or another Wine).\n";
} else {
    print "Play by running Lithtech.exe -cmdfile tron20cmds.txt in CrossOver (or another\n",
          "Wine): Tron.exe, the retail launcher, wants the retail setup's registry values.\n";
}
exit 0;
__END__
