#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

typedef uint32_t word;
typedef unsigned short half;
typedef unsigned char  byte;

#define swap_w(w) (BE)?(w):(word)((((w)<<24)&0xff000000)|(((w)<< 8)&0x00ff0000)|\
                            (((w)>> 8)&0x0000ff00)|(((w)>>24)&0x000000ff))
#define swap_h(w) (BE)?(w):((((w)<< 8)&0x0000ff00)|(((w)>> 8)&0x000000ff))

#define OP_H 0x0000
#define OP_I 0x0001
#define OP_M 0x0002
#define OP_C 0x0004
#define OP_F 0x0008

#define MAXSIZE (3 * 1024 * 1024)

struct firminfo {
  word info_ver, firmid;
  char firmname[32];
  char subver[32];
  half ver_major, ver_minor;
  half build;
  byte year, mon, day, hour, min, sec;
  word size;
  word chksum;
  word kernel_offset, kernel_size;
  word initrd_offset, initrd_size;
};

unsigned int BE; //endian 1:big 0:little

void showinfo(struct firminfo *pf, word sum, char *fni)
{
  printf("---- firmimg_file information ----\n");
  printf("filename : %s\n", fni);
  printf("checksum : %08X\n", sum);
  printf("---- firmware information ----\n");
  printf("info_ver : %08X\n", swap_w(pf->info_ver));
  printf("firmid   : %08X\n", swap_w(pf->firmid));
  printf("firmname : %s\n", pf->firmname);
  printf("subver   : %s\n", pf->subver);
  printf("version  : %d.%02d, build %04X\n",
          swap_h(pf->ver_major), swap_h(pf->ver_minor), swap_h(pf->build));
  printf("date     : %04d/%02d/%02d %02d:%02d:%02d\n",
          pf->year + 1900, pf->mon,  pf->day, pf->hour, pf->min,  pf->sec);
  printf("firmsize : %08X\n", swap_w(pf->size));
  printf("checksum : %08X\n", swap_w(pf->chksum));
  printf("kernel   : offset %08X, size %08X\n",
          swap_w(pf->kernel_offset), swap_w(pf->kernel_size));
  printf("initrd   : offset %08X, size %08X\n",
          swap_w(pf->initrd_offset), swap_w(pf->initrd_size));
}

int rd_file(char *fn, void *buf, int max, int min)
{
  FILE *fp;
  int r;

  if ((fp = fopen(fn, "r")) == NULL) {
    fprintf(stderr, "Cannot open '%s' for reading\n", fn);
    return 0;
  }
  r = fread(buf, 1, max, fp);
  fclose (fp);

  if (r < min) {
    fprintf(stderr, "Cannot read '%s'\n", fn);
    return 0;
  }
  return r;
}

int wr_file(char *fn, void *buf, size_t len)
{
  FILE *fp;

  if ((fp = fopen(fn, "w+")) == NULL) {
    fprintf(stderr, "Cannot open '%s' for writing\n", fn);
    return 1;
  }
  if (fwrite(buf, 1, len, fp) != len) {
    fprintf(stderr, "Cannot write '%s'\n", fn);
    fclose(fp);
    return 1;
  }
  fclose(fp);
  return 0;
}

void init_fi(struct firminfo *pf)
{
  struct tm *t;
  time_t tt;

  time(&tt);
  t = localtime(&tt);

  pf->info_ver = swap_w((word)0x00000001);
  pf->firmid = swap_w((word)0x00002001);
  strcpy(pf->firmname, "HD-HTGL(YOSHIMUNE)");
  strcpy(pf->subver, "FLASH 2.0");
  pf->ver_major = swap_h((half)0x0001);
  pf->ver_minor = swap_h((half)0x0004);
  pf->build = swap_h((half)0x0000);
  pf->chksum = 0;
  pf->year = t->tm_year;
  pf->mon  = t->tm_mon + 1;
  pf->day  = t->tm_mday;
  pf->hour = t->tm_hour;
  pf->min  = t->tm_min;
  pf->sec  = t->tm_sec;
}

int main(int argc, char *argv[])
{
  struct firminfo *fi;
  word *wp, w, sum;
  word c, fs;
  byte *buf;

  char *fni, *fnk, *fnr, *fnf;
  char *p;
  int m;

  m = OP_H;
  fni = NULL;
  fnk = NULL;
  fnr = NULL;

  /*-------- initialize --------*/

  if ((buf = malloc(MAXSIZE + 4)) == NULL) {
    fprintf(stderr, "Memory allocation error\n");
    return 1;
  }
  fi = (struct firminfo *)buf;
  init_fi(fi);

  c = 0x01000000;
  BE = *(byte *)(&c);

  /*-------- arguments --------*/

  for( int i=1; i<argc; i++ ) {
    if (*(p = argv[i]) == '-') {
      while(*++p != 0) {
        switch(*p) {
        case 'i': m |= OP_I; break;
        case 'c': m |= OP_C; break;
        case 'm': m |= OP_M; break;
        case 'k': fnk = argv[++i]; break;
        case 'r': fnr = argv[++i]; break;
        case 'f': fnf = argv[++i]; m |= OP_F; break;
        case 'h': m = OP_H; break;
        default : m = OP_H;
        }
      }
    }
    else
      fni = argv[i];
  }

  if ((m == OP_H) && (fni != NULL))
    m = OP_I;

  if ((m == OP_H) || (fni == NULL) || ((m & OP_C) && (m & OP_M)) ||
      ((m & (OP_C | OP_M)) && (fnk == NULL) && (fnr == NULL))) {
    fprintf(stderr, "Usage: %s [-icmkr] <firmimg_filename>\n", argv[0]);
    fprintf(stderr, "\t-i : show firmimg_file information\n");
    fprintf(stderr, "\t-c : cut out from firmimg_file\n");
    fprintf(stderr, "\t-m : merge into firmimg_file\n");
    fprintf(stderr, "\t-k <filename> : kernel image filename\n");
    fprintf(stderr, "\t-r <filename> : ramdisk image filename\n");
    fprintf(stderr, "\t-f <filename> : copy headers from file\n");
    fprintf(stderr, "\t-h : show this help message\n");
    return 1;
  }

  /*-------- file read --------*/

  if (m & OP_M) {
    if (m & OP_F) {
      if ((c = rd_file(fnf, fi, sizeof(*fi), 1)) == 0) {
        free(buf);
        return 1;
      }
      fi->chksum = 0;
    }
    fs = sizeof(*fi);
    if ((c = rd_file(fnk, &buf[fs], MAXSIZE - fs, 1)) == 0) {
      free(buf);
      return 1;
    }
    fi->kernel_offset = swap_w(fs);
    fi->kernel_size = swap_w(c);
    fs += c;
    if ((c = rd_file(fnr, &buf[fs], MAXSIZE + 4 - fs, 1)) == 0) {
      free(buf);
      return 1;
    }
    fi->initrd_offset = swap_w(fs);
    fi->initrd_size = swap_w(c);
    fs += c;
    fi->size = swap_w(fs);
    if (fs > MAXSIZE) {
      fprintf(stderr, "Firmware size overflow (maximum %d bytes)\n",
              MAXSIZE - sizeof(*fi));
      free(buf);
      return 1;
    }
  }
  else {
    if ((fs = rd_file(fni, buf, MAXSIZE, sizeof(*fi))) == 0) {
      free(buf);
      return 1;
    }
  }

  /*-------- checksum --------*/

  sum = 0;
  wp = (word *)buf;
  for(c = 0; c < (fs - 3); c += 4) {
    w = *wp++;
    sum += swap_w(w);
  }
  if (c < fs) {
    w = swap_w(*wp);
    switch(fs - c) {
    case 1: sum += (w & 0xff000000); break;
    case 2: sum += (w & 0xffff0000); break;
    case 3: sum += (w & 0xffffff00); break;
    }
  }

  if (m & OP_M) {
    fi->chksum = swap_w(-sum);
    sum = 0;
  }

  /*-------- file write --------*/

  if (m & OP_C) {
    if (fnk != NULL) {
      if (wr_file(fnk, &buf[swap_w(fi->kernel_offset)],
              swap_w(fi->kernel_size)) != 0) {
        free(buf);
        return 1;
      }
    }
    if (fnr != NULL) {
      if (wr_file(fnr, &buf[swap_w(fi->initrd_offset)],
               swap_w(fi->initrd_size)) != 0) {
        free(buf);
        return 1;
      }
    }
  }
  else if (m & OP_M) {
    if (fni != NULL) {
      if (wr_file(fni, buf, fs) != 0) {
        free(buf);
        return 1;
      }
    }
  }

  /*-------- show status --------*/

  if (m & OP_I) {
    showinfo(fi, sum, fni);
  }

  free(buf);
  return 0;
}
