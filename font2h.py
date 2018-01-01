#!/usr/bin/env python
# convert Espruino font to header file

import sys, base64, re

stdin = sys.stdin.read()

match = re.search(r'var font = atob\("([^"]*)"', stdin)
if match:
     font = base64.b64decode(match.group(1))
else:
    sys.stderr.write('no font data found\n')
    sys.exit(1)

match = re.search(r'var widths = atob\("([^"]*)"', stdin)
if match:
     widths = base64.b64decode(match.group(1))
else:
    sys.stderr.write('no widths data found\n')
    sys.exit(1)

match = re.search(r'this.setFontCustom\(font, (\d+), widths, (\d+)\);', stdin)
if match:
    startchar = int(match.group(1))
    height = int(match.group(2))
else:
    sys.stderr.write('no height/startchar data found\n')
    sys.exit(1)

if height != 8:
    sys.stderr.write('unsupported height (%d)\n' % height)
    sys.exit(1)

sys.stdout.write('#ifndef _FONT_H_\n#define _FONT_H_\n\n')

sys.stdout.write('#define FONT_MINCHAR\t%d\n' % startchar)
sys.stdout.write('#define FONT_MAXCHAR\t127\n\n')

sys.stdout.write('static const unsigned char font_data[] PROGMEM = {')

for i in range(len(font)):
    sys.stdout.write('%s0x%02x,' % ('\n\t' if i % 8 == 0 else ' ', ord(font[i])))

sys.stdout.write('\n};\n\n')

sys.stdout.write('static const unsigned char font_width[] PROGMEM = {')

for i in range(len(widths)):
    sys.stdout.write('%s0x%02x,' % ('\n\t' if i % 8 == 0 else ' ', ord(widths[i])))

sys.stdout.write('\n};\n\n')

sys.stdout.write('static const unsigned int font_offset[] PROGMEM = {')

ofs = 0
for i in range(len(widths)):
    sys.stdout.write('%s%3d,' % ('\n\t' if i % 8 == 0 else ' ', ofs))
    ofs += ord(widths[i])

sys.stdout.write('\n};\n\n')

sys.stdout.write('#endif /* _FONT_H_ */\n')
