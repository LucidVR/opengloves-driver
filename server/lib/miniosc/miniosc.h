

#ifndef _MINIOSC_H
#define _MINIOSC_H

/*
 miniosc.h - a small single-file header OSC library (including sockets) for C.
  It only supports a subset of the OSC protocol. #bundle, the basic types:
  i, f, s, and b and extended types I, F, N, and T, and t, c and r. Where c and
  r are aliased to i.

 Tested 2022-02-24 working with TouchOSC input and output (well except for the
 extended functionality).  While basic bounds checking is happening it is
 likely that there are flaws with this.  This library has not yet been
 fuzzed.
 Tested working on Linux and Windows 2022-02-24

 Copyright 2022 <>< cnlohr - you may use this file freely.

 It may be licensed under the MIT/x11, NewBSD, CC0 or Public Domain where applicable.
*/

#ifndef MINIOSCBUFFER
#define MINIOSCBUFFER 1536
#endif

#define MINIOSC_ERROR_SOCKET -1
#define MINIOSC_ERROR_BIND -2
#define MINIOSC_ERROR_CONNECT -2
#define MINIOSC_ERROR_PARAMS -3
#define MINIOSC_ERROR_OVERFLOW -4
#define MINIOSC_ERROR_TRANSPORT -5  // May not be fatal.
#define MINIOSC_ERROR_PROTOCOL -6
#define MINIOSC_ERROR_EMPTY -7

typedef struct {
  int sock;
  int portout;
} miniosc;

typedef struct {
  char *bundleplace;
  char bundledata[MINIOSCBUFFER];
} mobundle;

// Initialize a miniosc.  It mallocs the data in the return structure.  If there was an
// error, it will return NULL, and minioscerrorcode will be populated if it was nonnull.
miniosc *minioscInit(int portin, int portout, const char *addressout, int *minioscerrorcode);

// Send an OSC message, this will pull the types off of varargs.
//  'i', 'c' and 'r': send an int, include an int in your varargs.
//  'f': send a float, include a float (or rather auto-promoted double) to your varargs.
//  's': send a string, include a char * in your varargs.
//  'b': send a blob, include an int and then a char * in your varargs.
//  'T', 'I', 'F' and 'N' have no parameters.
// You must prefix your address with '/' and you must prefix type with ','
int minioscSend(miniosc *osc, const char *address, const char *type, ...);
int minioscBundle(mobundle *mo, const char *address, const char *type, ...);

// Actually send a bundle.
int minioscSendBundle(miniosc *osc, mobundle *mo);

// Poll for an OSC message.
//   - If the value is negative, there was an error, it reports the error.
//   - If the value is zero, no messages were received, it was timed out.
//   - If the value is positive, it reports the number of messages received and processed.
int minioscPoll(miniosc *osc, int timeoutms, void (*callback)(const char *address, const char *type, void **parameters));

// Close the socket. Free the memory.
void minioscClose(miniosc *osc);

#ifdef MINIOSC_IMPLEMENTATION

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(WINDOWS) || defined(WIN32) || defined(WIN64) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS)
#define MINIOSCWIN 1
#endif

#ifdef MINIOSCWIN
#include <WinSock2.h>
#include <Windows.h>
#define socklen_t int
#define MSG_NOSIGNAL 0
#else
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
#define closesocket close
#endif

// Internal functions
static int _minioscAppend(char *start, char **ptr, int max_len, int datalen, const char *data) {
  int quad_byte_length = ((datalen + 3) & (~3));
  if (*ptr - start + quad_byte_length >= max_len) return MINIOSC_ERROR_OVERFLOW;
  memcpy(*ptr, data, datalen);
  memset(*ptr + datalen, 0, quad_byte_length - datalen);
  *ptr += quad_byte_length;
  return 0;
}

static int _minioscGetQBL(const char *start, char **here, int length) {
  // Note this function does not round up so it can be used with blobs.
  const char *h = *here;

  if (h - start >= length) return -1;
  while (*h) {
    h++;
    if (h - start >= length) return -1;
  }
  h++;

  int thislen = h - start;
  int quad_byte_length = ((thislen + 3) & (~3));
  *here += quad_byte_length;
  return thislen;
}

miniosc *minioscInit(int portin, int portout, const char *addressout, int *minioscerrorcode) {
#ifdef MINIOSCWIN
  WSADATA rep;
  if (WSAStartup(MAKEWORD(2, 2), &rep) != 0) {
    // Really, this code can't be executed past windows 3.11, windows for workgroups
    // but I like having it here, as a faint reminder of the terrible world from
    // whence we've come.
    if (minioscerrorcode) *minioscerrorcode = MINIOSC_ERROR_SOCKET;
    return 0;
  }
#endif

  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (sock == -1) {
    if (minioscerrorcode) *minioscerrorcode = MINIOSC_ERROR_SOCKET;
    return 0;
  }

  if (portin) {
    struct sockaddr_in bindaddy;

    // We have no remote address, therefore we are bound.
    bindaddy.sin_family = AF_INET;
    bindaddy.sin_addr.s_addr = INADDR_ANY;
    bindaddy.sin_port = htons(portin);

    // Allow address reuse. Depending on OS, we may or may not get a message.
    // Consider this.
    // int opt = 1;
    // setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, ( const char * ) &opt, sizeof( opt ) );

    // Bind the socket with the server address
    if (bind(sock, (const struct sockaddr *)&bindaddy, sizeof(bindaddy)) < 0) {
      closesocket(sock);
      if (minioscerrorcode) *minioscerrorcode = MINIOSC_ERROR_BIND;
      return 0;
    }
  }

  if (addressout && portout) {
    struct sockaddr_in peeraddy;
    // We have a remote address, therefore we are outgoing.
    peeraddy.sin_family = AF_INET;
    peeraddy.sin_port = htons(portout);
    peeraddy.sin_addr.s_addr = inet_addr(addressout);

    // If you find the OS's buffer is insufficient, we could override
    // setsockopt( m_sockfd, SOL_SOCKET, SO_RCVBUF, ( const char * ) &n, sizeof( n ) ) ...
    //
    // Possible future feature: bind to a specific network device.
    // setsockopt( m_sockfd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof( ifr ) ) ...

    if (connect(sock, (const struct sockaddr *)&peeraddy, sizeof(peeraddy)) < 0) {
      if (minioscerrorcode) *minioscerrorcode = MINIOSC_ERROR_CONNECT;
      closesocket(sock);
      return 0;
    }
  }

#ifndef MINIOSCWIN
  // Tricky: We want to make OSC use telephony QoS / ToS.  This
  // will reduce its latency in places where it would be inappropriate
  // to queue too many packets.
  // Officially, 46<<2; //46 = Telephony, priority.
  // But, experimentally, most wifi drivers respond better with 0xff.
  int tos_local = 0xff;
  socklen_t len = sizeof(tos_local);
  if (setsockopt(sock, IPPROTO_IP, IP_TOS, &tos_local, sizeof(tos_local))) {
    // Not a big deal if this fails.  We can fail silently.  Alternatively
    // we could after setting also check to make sure it was actually
    // set by calling getsockopt.
  }
#endif
  if (minioscerrorcode) *minioscerrorcode = 0;
  miniosc *ret = (miniosc*)malloc(sizeof(miniosc));
  ret->sock = sock;
  return ret;
}

static int minioscEncodeInternal(char *buffer, char **bptr, int mob, const char *address, const char *type, va_list ap) {
  int err;
  if (!address || address[0] != '/' || !type || type[0] != ',') return MINIOSC_ERROR_PARAMS;

  if ((err = _minioscAppend(buffer, bptr, MINIOSCBUFFER, strlen(address) + 1, address))) return err;
  if ((err = _minioscAppend(buffer, bptr, MINIOSCBUFFER, strlen(type) + 1, type))) return err;

  const char *t;
  char c;
  for (t = type + 1; (c = *t); t++) {
    switch (c) {
      case 'i':
      case 'c':
      case 'r': {
        int i = va_arg(ap, unsigned int);
        i = htonl(i);
        if ((err = _minioscAppend(buffer, bptr, MINIOSCBUFFER, 4, (const char *)&i))) return err;
        break;
      }
      case 'f': {
        float f = va_arg(ap, double);  // Quirk: var args in C use doubles not floats because of automatic type promotion.

        // Flip endian of float.
        uint32_t *fi = (uint32_t *)&f;
        *fi = htonl(*fi);

        if ((err = _minioscAppend(buffer, bptr, MINIOSCBUFFER, 4, (const char *)&f))) return err;
        break;
      }
      case 's': {
        const char *st = va_arg(ap, const char *);
        int sl = strlen(st) + 1;
        if ((err = _minioscAppend(buffer, bptr, MINIOSCBUFFER, sl, st))) return err;
        break;
      }
      case 'b': {
        int len = va_arg(ap, int);
        const void *st = va_arg(ap, const void *);
        int lensend = htonl(len);
        if ((err = _minioscAppend(buffer, bptr, MINIOSCBUFFER, 4, (const char *)&lensend))) return err;
        if ((err = _minioscAppend(buffer, bptr, MINIOSCBUFFER, len, (const char*)st))) return err;
        break;
      }
      case 'F':
      case 'I':
      case 'N':
      case 'T':
        // No data encoded for T, F or N.
        break;
      default:
        return MINIOSC_ERROR_PARAMS;
    }
  }
  return 0;
}

static int minioscSendData(miniosc *osc, int sendlen, const char *buffer) {
  if (send(osc->sock, buffer, sendlen, MSG_NOSIGNAL) != sendlen) return MINIOSC_ERROR_TRANSPORT;
  return 0;
}

int minioscSend(miniosc *osc, const char *address, const char *type, ...) {
  int err;
  char buffer[MINIOSCBUFFER];
  char *bptr = buffer;

  va_list args;
  va_start(args, type);
  int ret = minioscEncodeInternal(buffer, &bptr, MINIOSCBUFFER, address, type, args);
  va_end(args);
  if (ret) return ret;

  // We have constructed a valid packet.  Send it.
  int sendlen = bptr - buffer;
  return minioscSendData(osc, sendlen, buffer);
}

int minioscBundle(mobundle *mo, const char *address, const char *type, ...) {
  if ((void *)mo->bundleplace == (void *)1) return MINIOSC_ERROR_OVERFLOW;
  if (!mo->bundleplace) {
    memcpy(mo->bundledata, "#bundle\0\0\0\0\0\0\0\0", 16);  // 0 timestamp.
    mo->bundleplace = mo->bundledata + 16;
  }
  va_list args;
  va_start(args, type);
  uint32_t *bpl = (uint32_t *)mo->bundleplace;
  mo->bundleplace += 4;  // Reserve 4 bytes for bundle length.
  int ret = minioscEncodeInternal(mo->bundledata, &mo->bundleplace, MINIOSCBUFFER, address, type, args);
  va_end(args);
  if (ret == 0) {
    // No error, write in length of message.
    int len = mo->bundleplace - (char *)bpl - 4;
    if (len) {
      (*bpl) = htonl(len);
    }
  }
  return ret;
}

// Actually send a bundle.
int minioscSendBundle(miniosc *osc, mobundle *mo) {
  // We have constructed a valid packet.  Send it.
  if (!mo->bundleplace || (void *)mo->bundleplace == (void *)1) return MINIOSC_ERROR_EMPTY;
  int sendlen = mo->bundleplace - mo->bundledata;
  return minioscSendData(osc, sendlen, mo->bundledata);
}

static int minioscProcess(char *buffer, char **eptr, int r, void (*callback)(const char *address, const char *type, void **parameters)) {
  const char *name = *eptr;
  int namelen = _minioscGetQBL(name, eptr, r);
  const char *type = *eptr;
  int typelen = _minioscGetQBL(type, eptr, r);

  if (namelen <= 0 || typelen <= 0 || name[0] != '/' || type[0] != ',') return MINIOSC_ERROR_PROTOCOL;

  const char *parameters[MINIOSCBUFFER / 4];  // Worst-case scenario: all blobs.
  int p = 0;
  const char *ct;

  for (ct = type + 1; *ct; ct++) {
    switch (*ct) {
      case 'i':
      case 'c':
      case 'r': {
        if (*eptr - buffer + 4 > r) return MINIOSC_ERROR_PROTOCOL;
        *((int *)*eptr) = htonl(*((int *)*eptr));
        parameters[p++] = *eptr;
        *eptr += 4;
        break;
      }
      case 'f': {
        if (*eptr - buffer + 4 > r) return MINIOSC_ERROR_PROTOCOL;
        *((int *)*eptr) = htonl(*((int *)*eptr));
        parameters[p++] = *eptr;
        *eptr += 4;
        break;
      }
      case 's': {
        char *st = *eptr;
        int sl = _minioscGetQBL(buffer, eptr, r);
        if (sl < 0) MINIOSC_ERROR_PROTOCOL;
        if (sl == 0) {
          parameters[p] = "\0\0\0\0";
          *eptr += 4;
        } else
          parameters[p++] = st;
        break;
      }
      case 'b': {
        if (*eptr - buffer + 4 > r) return MINIOSC_ERROR_PROTOCOL;
        parameters[p++] = *eptr;
        *eptr += 4;
        char *st = *eptr;
        int sl = _minioscGetQBL(buffer, eptr, r);
        if (sl < 0) return MINIOSC_ERROR_PROTOCOL;
        if (sl == 0)
          parameters[p++] = "\0\0\0\0";
        else
          parameters[p++] = st;
        break;
      }
      case 'T':
      case 'I':
      case 'F':
      case 'N':
        break;
      default:
        return MINIOSC_ERROR_PARAMS;
    }
  }
  callback(name, type, (void **)parameters);
}

int minioscPoll(miniosc *osc, int timeoutms, void (*callback)(const char *address, const char *type, void **parameters)) {
  int r;
  // Do a poll.

#ifdef MINIOSCWIN
  TIMEVAL timeout = {0};
  timeout.tv_usec = timeoutms * 1000;
  FD_SET fds;
  FD_ZERO(&fds);
  FD_SET(osc->sock, &fds);
  r = select(1, &fds, 0, 0, &timeout);
  if (r == 0) r = FD_ISSET(osc->sock, &fds);
#else
  struct pollfd pfd = {0};
  pfd.fd = osc->sock;
  pfd.events = POLLRDNORM;
  r = poll(&pfd, 1, timeoutms);
#endif

  if (r < 0) return MINIOSC_ERROR_TRANSPORT;

  // No data.
  if (r == 0) return 0;

  char buffer[MINIOSCBUFFER];
  r = recv(osc->sock, buffer, sizeof(buffer), MSG_NOSIGNAL);
  if (r <= 8) return MINIOSC_ERROR_TRANSPORT;

  char *eptr = buffer;
  int rxed = 0;
  int err = 0;

  if (memcmp(buffer, "#bundle", 8) == 0) {
    // Bundle
    eptr += 16;  // "#bundle\0" + timecode (we ignore timecode)
    while (eptr - buffer < r - 4) {
      int32_t tolen = htonl(*((int32_t *)eptr));
      char *ep2 = eptr;
      err = minioscProcess(eptr, &ep2, tolen, callback);
      rxed++;
    }
  } else {
    err = minioscProcess(buffer, &eptr, r, callback);
    rxed = 1;
  }
  return rxed;
}

void minioscClose(miniosc *osc) {
  closesocket(osc->sock);
  osc->sock = 0;
  free(osc);
}

#endif

#endif
