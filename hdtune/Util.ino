//
// Misc utilities
//

// convert four hex characters to an unsigned int
unsigned int hex2uint16(const char *p)
{
  char c = *p;
  unsigned int i = 0;
  for (char n = 0; c && n < 4; c = *(++p)) {
    if (c >= 'A' && c <= 'F') {
      c -= 7;
    } 
    else if (c>='a' && c<='f') {
      c -= 39;
    } 
    else if (c == ' ') {
      continue;
    } 
    else if (c < '0' || c > '9') {
      break;
    }
    i = (i << 4) | (c & 0xF);
    n++;
  }
  return i;
}

// convert two hex characters to an unsigned char
unsigned char hex2uint8(const char *p)
{
  unsigned char c1 = *p;
  unsigned char c2 = *(p + 1);
  if (c1 >= 'A' && c1 <= 'F')
    c1 -= 7;
  else if (c1 >='a' && c1 <= 'f')
    c1 -= 39;
  else if (c1 < '0' || c1 > '9')
    return 0;

  if (c2 >= 'A' && c2 <= 'F')
    c2 -= 7;
  else if (c2 >= 'a' && c2 <= 'f')
    c2 -= 39;
  else if (c2 < '0' || c2 > '9')
    return 0;

  return c1 << 4 | (c2 & 0xf);
}

