#!/usr/bin/env python
# convert text messages to array in progmem

import sys

msgs = sys.stdin.readlines()

sys.stdout.write('#ifndef _MSGS_H_\n#define _MSGS_H_\n\n')

for i in range(len(msgs)):
    m = msgs[i].strip()
    sys.stdout.write('static const char msg%03d[] PROGMEM = "%s";\n' % (i, m))

sys.stdout.write('\n')

sys.stdout.write('static const char * const msgs[] PROGMEM = {')

for i in range(len(msgs)):
    sys.stdout.write('%smsg%03d,' % ('\n\t' if i % 3 == 0 else ' ', i))

sys.stdout.write('\n};\n\n')

sys.stdout.write('#define MSGS\t%d\n\n' % len(msgs))

sys.stdout.write('#endif /* _MSGS_H_ */\n')
