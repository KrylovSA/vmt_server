#ifndef ZPIPE_H
#define ZPIPE_H
int def(FILE *source, FILE *dest, int level);
int inf(FILE *source, FILE *dest);
void zerr(int ret);
#endif
