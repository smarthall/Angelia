
int getibaud(const struct termios *termios_p);
int getobaud(const struct termios *termios_p);

#define CHECK_FLAG(bf, f) (((bf) & (f)) == (f))

