// mp3acmtest.cpp -- a test of the modern SndDrv.dll's MP3 codec
// (mp3acm.cpp, MODERN.md X-2), outside the game. build/mp3acmtest.sh
// builds it with Zig's C compiler and runs it under Wine on every WAV file
// of Sound.rez. For each WAV file in the list (argv[1]) that is MP3:
//  A. "whole": acmFormatSuggest and acmStreamOpen with NO driver handle
//     (ACM searches its drivers: what a caller with no driver of its own,
//     such as DirectMusic, gets), one convert
//     with all of the data (as SndDrv's InitSampleFromFile does);
//  B. "streamed": through the opened driver (as SndDrv does), blocks of
//     acmStreamSize(8192, DESTINATION), 8192-byte output (as
//     WaveFile::ReadCompressed), then again from the start with ACM_STREAMCONVERTF_START
//     (WaveFile::Cue), the last block with ACM_STREAMCONVERTF_END. A and B
//     must give the same PCM.
// Prints one line per failure and a summary.
#include <windows.h>
#include <mmreg.h>
#include <msacm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Buf { unsigned char* p; size_t n, cap;
  Buf():p(0),n(0),cap(0){}
  ~Buf(){ free(p); }
  void reserve(size_t c){ if(c>cap){ size_t k=cap?cap:4096; while(k<c)k*=2; p=(unsigned char*)realloc(p,k); cap=k; } }
  void append(const unsigned char* s,size_t m){ reserve(n+m); memcpy(p+n,s,m); n+=m; }
  void resize(size_t m){ reserve(m); if(m>n) memset(p+n,0,m-n); n=m; }
  void assign(const unsigned char* a,const unsigned char* b){ n=0; append(a,b-a); }
  size_t size() const { return n; } bool empty() const { return !n; }
  unsigned char& operator[](size_t i){ return p[i]; }
  void clear(){ n=0; }
};
static bool Same(const Buf& a,const Buf& b){ return a.n==b.n && !memcmp(a.p,b.p,a.n); }

static FILE* g_pLogFile = NULL;
#define LOG_WRITE(...) ((void)0)
#include "mp3acm.cpp"

static bool ReadWav(const char* p, Buf& fmt, Buf& data)
{
	FILE* f = fopen(p, "rb"); if (!f) return false;
	Buf b; unsigned char t[65536]; size_t n;
	while ((n = fread(t, 1, sizeof t, f)) > 0) b.append(t, n);
	fclose(f);
	if (b.size() < 12 || memcmp(&b[0], "RIFF", 4)) return false;
	size_t i = 12;
	while (i + 8 <= b.size()) {
		unsigned int len = *(unsigned int*)&b[i + 4];
		if (!memcmp(&b[i], "fmt ", 4)) fmt.assign(b.p + i + 8, b.p + i + 8 + len);
		if (!memcmp(&b[i], "data", 4)) data.assign(b.p + i + 8, b.p + i + 8 + (len < b.size() - i - 8 ? len : b.size() - i - 8));
		i += 8 + len + (len & 1);
	}
	return fmt.size() >= 16 && !data.empty();
}

int main(int argc, char** argv)
{
	HACMDRIVERID hId = Mp3Acm_Register();
	if (!hId) { printf("register failed\n"); return 1; }
	HACMDRIVER hDrv; if (acmDriverOpen(&hDrv, hId, 0)) { printf("acmDriverOpen failed\n"); return 1; }
	FILE* list = fopen(argv[1], "r");
	char line[1024]; int nFiles = 0, nFail = 0; double fSecs = 0, fWorst = 0; char szWorst[1024] = "";
	while (fgets(line, sizeof line, list)) {
		line[strcspn(line, "\r\n")] = 0;
		Buf fmtb, data;
		if (!ReadWav(line, fmtb, data)) { printf("FAIL %s: not read\n", line); ++nFail; continue; }
		fmtb.resize(fmtb.size() < sizeof(WAVEFORMATEX) ? sizeof(WAVEFORMATEX) : fmtb.size());
		WAVEFORMATEX* src = (WAVEFORMATEX*)&fmtb[0];
		if (src->wFormatTag != WAVE_FORMAT_MPEGLAYER3) continue;
		++nFiles;
		WAVEFORMATEX pcm; memset(&pcm, 0, sizeof pcm); pcm.wFormatTag = WAVE_FORMAT_PCM;
		// A: no driver handle
		MMRESULT r = acmFormatSuggest(NULL, src, &pcm, sizeof pcm, ACM_FORMATSUGGESTF_WFORMATTAG);
		if (r) { printf("FAIL %s: acmFormatSuggest(NULL) %u\n", line, r); ++nFail; continue; }
		HACMSTREAM h;
		r = acmStreamOpen(&h, NULL, src, &pcm, NULL, 0, 0, 0);
		if (r) { printf("FAIL %s: acmStreamOpen(NULL) %u\n", line, r); ++nFail; continue; }
		double dur = (double)data.size() / src->nAvgBytesPerSec;
		DWORD cbDst = (DWORD)(dur * pcm.nAvgBytesPerSec * 1.2) + 65536;
		Buf whole; whole.resize(cbDst);
		ACMSTREAMHEADER hdr; memset(&hdr, 0, sizeof hdr); hdr.cbStruct = sizeof hdr;
		hdr.pbSrc = &data[0]; hdr.cbSrcLength = data.size(); hdr.pbDst = &whole[0]; hdr.cbDstLength = cbDst;
		acmStreamPrepareHeader(h, &hdr, 0);
		r = acmStreamConvert(h, &hdr, 0);
		acmStreamUnprepareHeader(h, &hdr, 0);
		acmStreamClose(h, 0);
		if (r) { printf("FAIL %s: convert %u\n", line, r); ++nFail; continue; }
		whole.resize(hdr.cbDstLengthUsed);
		double pcmSecs = (double)whole.size() / pcm.nAvgBytesPerSec;
		fSecs += pcmSecs;
		double diff = pcmSecs - dur; if (diff < 0) diff = -diff;
		// B: streamed, twice (the second after a reset)
		r = acmStreamOpen(&h, hDrv, src, &pcm, NULL, 0, 0, 0);
		if (r) { printf("FAIL %s: acmStreamOpen(driver) %u\n", line, r); ++nFail; continue; }
		DWORD cbBlock = 0; acmStreamSize(h, 8192, &cbBlock, ACM_STREAMSIZEF_DESTINATION);
		Buf sbuf, dbuf; sbuf.resize(cbBlock); dbuf.resize(8192);
		memset(&hdr, 0, sizeof hdr); hdr.cbStruct = sizeof hdr;
		hdr.pbSrc = &sbuf[0]; hdr.cbSrcLength = cbBlock; hdr.pbDst = &dbuf[0]; hdr.cbDstLength = 8192;
		acmStreamPrepareHeader(h, &hdr, 0);
		bool bad = false;
		for (int pass = 0; pass < 2 && !bad; ++pass) {
			bool start = pass == 1;
			Buf out;
			size_t pos = 0;
			// pass 0 stops halfway; pass 1 starts again (START) and must not
			// give anything of pass 0
			while (pos < data.size() || pass == 0) {
				size_t n = data.size() - pos; if (n > cbBlock) n = cbBlock;
				if (pass == 0 && pos >= data.size() / 2) break;  // stop halfway: the reset must drop the rest
				memcpy(&sbuf[0], &data[pos], n); pos += n;
				hdr.cbSrcLength = n;
				{ MMRESULT rc = acmStreamConvert(h, &hdr, (start ? ACM_STREAMCONVERTF_START : 0) | (pass == 1 && pos >= data.size() ? ACM_STREAMCONVERTF_END : 0)); start = false; if (rc) { printf("  convert error %u at %u (pass %d, block %u)\n", rc, (unsigned)pos, pass, (unsigned)cbBlock); bad = true; break; } }
				out.append(dbuf.p, hdr.cbDstLengthUsed);
			}
			if (pass == 1) {
				// drain what the codec still holds
				for (int k = 0; k < 1000; ++k) {
					hdr.cbSrcLength = 0;
					{ MMRESULT rc = acmStreamConvert(h, &hdr, 0); if (rc) { printf("  drain error %u\n", rc); bad = true; break; } }
					if (!hdr.cbDstLengthUsed) break;
					out.append(dbuf.p, hdr.cbDstLengthUsed);
				}
				if (!Same(out, whole)) { printf("FAIL %s: streamed %u bytes, whole %u bytes%s\n", line, (unsigned)out.size(), (unsigned)whole.size(),
					out.size() == whole.size() ? " (different bytes)" : ""); bad = true; }
			}
		}
		acmStreamUnprepareHeader(h, &hdr, 0);
		acmStreamClose(h, 0);
		if (bad) { ++nFail; continue; }
		if (diff > fWorst) { fWorst = diff; strcpy(szWorst, line); }
		if (diff > 0.5) printf("NOTE %s: %.2f s of PCM for %.2f s of MP3 data\n", line, pcmSecs, dur);
	}
	printf("%d MP3 files, %d failed, %.0f s of PCM; largest length difference %.3f s (%s)\n", nFiles, nFail, fSecs, fWorst, szWorst);
	return nFail != 0;
}
