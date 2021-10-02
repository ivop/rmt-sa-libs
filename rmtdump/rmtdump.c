/* rmtdump -- dump contents of rmt (raster music tracker) files
 * by ivo van poorten, june 2012
 * not very neat, but it does/did the job of reverse-engineering
 * the file format
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAXBLOCKS 2

int i, b, c, start[MAXBLOCKS], end[MAXBLOCKS], length[MAXBLOCKS];
int channels, maxtracklength;
int speed[2];
int itable, ttableL, ttableH, song;
int ninstruments, ntracks;

#define me  argv[0]
#define MEM16LE(x)  (mem[x]+(mem[x+1]<<8))

unsigned char mem[65536], *p;
char s[256][3];
char *notes[] = { "C-", "C#", "D-", "D#", "E-",
    "F-", "F#", "G-", "G#", "A-", "A#", "B-" };
char o2l[256];

int main(int argc, char **argv) {
    FILE *f;

    for (i=0; i<256; i++) {
        snprintf(s[i], 3, "%02x", i);
    }
    s[255][0]='-';
    s[255][1]='-';

    if (argc != 2) {
        fprintf(stderr, "usage: rmtdump <file>\n");
        exit(1);
    }

    if (!(f = fopen(argv[1], "r"))) {
        fprintf(stderr, "%s: %s: %s\n", me, argv[1], strerror(errno));
        exit(1);
    }

    c  = fgetc(f);
    c += fgetc(f)<<8;

    if (c != 0xffff) {
        fprintf(stderr, "%s: expected 0xffff at start of file\n", me);
        exit(1);
    }

    b = 0;
    while ((c=fgetc(f)) != EOF) {
        start [b]  = c;
        start [b] += fgetc(f)<<8;
        end   [b]  = fgetc(f);
        end   [b] += fgetc(f)<<8;
        length[b]  = end[b]-start[b]+1;

        fread(mem+start[b], 1, length[b], f);
        b++;
    }
    fclose(f);

    for (i=0; i<b; i++) {
        printf("start: $%04x, end: $%04x, lenght: $%04x\n",
                                            start[i], end[i], length[i]);
    }

    p = mem+start[0];

    if (strncmp(p, "RMT", 3)) {
        fprintf(stderr, "%s: not an RMT file\n", me);
        exit(1);
    }

    channels = 4 + (p[3] == '8') * 4;

    printf("Channels: %i\n", channels);

    maxtracklength = p[4];
    speed[0]       = p[5];
    speed[1]       = p[6];

    printf("Max. Track Length: %i, Song speed: $%02x, "
           "Instrument speed: $%02x\n", maxtracklength, speed[0], speed[1]);

    itable  = MEM16LE(start[0]+8);
    ttableL = MEM16LE(start[0]+10);
    ttableH = MEM16LE(start[0]+12);
    song    = MEM16LE(start[0]+14);

    ninstruments = (ttableL-itable)/2;
    ntracks      =  ttableH-ttableL;

    printf("Number of instruments: %i, start at $%04x\n", ninstruments, itable);
    printf("Number of tracks: %i, lsb's start at $%04x, "
           "msb's at $%04x\n", ntracks, ttableL, ttableH);
    printf("Song starts at $%04x\n", song);

    printf("\nInstruments\n");
    for (i=0; i<ninstruments; i++) {
        int tend, trep, eend, erep;
        int tlen, tgo, elen, ego;
        b = MEM16LE(itable+i*2);
        if (!b) {
            printf("\nINSTRUMENT $%02x: empty\n", i);
            continue;
        }
        p = mem+b;
        printf("\nINSTRUMENT $%02x: ($%04x) size: %i bytes\n", i, b, p[2]+3);
        printf("\tAUDCTL $%02x\n", p[5]);
        printf("\tEFFECT\n\t\tdelay: $%02x, vibrato: %1x, fshift: $%02x\n",
                p[8], p[9], p[10]);

        tend = p[0];
        trep = p[1];
        eend = p[2];
        erep = p[3];
        tlen = tend-12+1;
        tgo  = trep-12;

        printf("\tTABLE OF NOTES\n\t\ttlen: $%02x, tgo: $%02x, "
               "tspd: $%02x, type: %s, mode: %s\n", tlen, tgo, (p[4]&0x3f)+1,
               p[4]&0x80 ? "freqs" : "notes",
               p[4]&0x40 ? "accum" : "add");
        printf("\t\t");
        for(c=12; c<=tend; c++) {
            printf("%02x ", p[c]);
        }
        printf("\n");

        elen = ((eend+3)-(tend+1))/3;
        ego  = (erep-(tend+1))/3;
        printf("\tENVELOPE\n\t\telen: $%02x, ego: $%02x, "
               "vslide: $%02x, vmin: $%1x\n", elen, ego, p[6], p[7]);

        if (channels==8) {
            printf("\t\t  VOLUME R: ");
            for(c=tend+1; c<=eend; c+=3) printf("%1x", p[c]>>4);
            printf("\n");
        }

        printf("\t\t  VOLUME L: ");
        for(c=tend+1; c<=eend; c+=3) printf("%1x", p[c]&0x0f);
        printf("\n");

        printf("\t\tDISTORTION: ");
        for(c=tend+1; c<=eend; c+=3) printf("%1x", p[c+1]&0x0e);
        printf("\n");

        printf("\t\t   COMMAND: ");
        for(c=tend+1; c<=eend; c+=3) printf("%1x", (p[c+1]>>4)&7);
        printf("\n");

        printf("\t\t        X/: ");
        for(c=tend+1; c<=eend; c+=3) printf("%1x", p[c+2]>>4);
        printf("\n");

        printf("\t\t        Y\\: ");
        for(c=tend+1; c<=eend; c+=3) printf("%1x", p[c+2]&0x0f);
        printf("\n");

        printf("\t\t    FILTER: ");
        for(c=tend+1; c<=eend; c+=3) printf("%c", p[c+1]&0x80 ? '*' : '.');
        printf("\n");

        printf("\t\tPORTAMENTO: ");
        for(c=tend+1; c<=eend; c+=3) printf("%c", p[c+1]&1 ? '*' : '.');
        printf("\n");
    }

    printf("\nTracks\n");
    for (i=0; i<ntracks; i++) {
        int L, H, A, duration;
        p = mem+ttableL+i;
        L = *p;
        p = mem+ttableH+i;
        H = *p;
        A = L+(H<<8);
        printf("\nTRACK $%02x ($%04x)\n", i, A);
        if (!A) {
            printf("\tempty\n");
            continue;
        }
        p = mem+A;
        duration = 0;
        memset(o2l,0,256);
        while (duration < maxtracklength) {
            int len, v, n, j, b2, b1;
            o2l[p-mem-A]=duration;
            b1=*p++;
            switch(b1&0x3f) {
            case 0x3d:      // set volume
                b2 = *p++;
                v = b1>>6;
                v+= (b2&3)<<2;
                printf("\t%02x: --- -- %1x\n", duration, v);
                duration++;
                break;
            case 0x3e:      // "rest"
                len = b1>>6;
                if (!len) {
                    len=*p++;
                }
                while(len--) {
                    printf("\t%02x: --- -- -\n", duration);
                    duration++;
                }
                break;
            case 0x3f:      // end, jump, set speed
                if (b1&0x80) {
                    if (b1==0xff) {
                        printf("\tend\n");
                    } else {
                        b2=*p++;
                        printf("\tjump %02x\n", o2l[b2]);
                    }
                    duration=maxtracklength;
                } else {    // set speed
                    b2=*p++;
                    printf("\tset speed %02x\n", b2);
                }
                break;
            default:        // note+volume
                b2 = *p++;
                v = b1>>6;
                v+= (b2&3)<<2;
                n = b1&0x3f;
                j = b2>>2;
                printf("\t%02x: %2s%1i %02x %1x\n", duration, notes[n%12], (n/12)+1, j, v);
                duration++;
                break;
            }
        }
    }

    printf("\nSong\n");
    if (channels==4) {
        printf("\n    L1 L2 L3 L4\n---------------\n");
    }else{
        printf("\n    L1 L2 L3 L4 R1 R2 R3 R4\n---------------------------\n");
    }

    for (p=mem+song, i=0; p<mem+end[0]+1; i++) {
        printf("%02x: ", i);
        if (p[0]==0xfe) {
            printf("goto line --> %02x", ((p[2]+(p[3]<<8))-song)/channels);
            if (channels==8) p+=4;
        }else {
            printf("%s %s %s %s", s[p[0]], s[p[1]], s[p[2]], s[p[3]]);
            if (channels==8) {
                printf(" %s %s %s %s", s[p[4]], s[p[5]], s[p[6]], s[p[7]]);
                p+=4;
            }
        }
        p+=4;
        printf("\n");
    }

    for (p=mem+start[1], i=-1; p<mem+end[1]+1; i++) {
        int v=strlen(p);
        if (i==-1) printf("\nSong name: ");
        else       printf("%2i: ", i);
        printf("%s\n", p);
        p+=v+1;
    }
}
