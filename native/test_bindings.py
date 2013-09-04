import sys

sys.path.insert(0, 'build/lib.linux-x86_64-2.7')

import wsp

print wsp.WhisperArchiveInfo
w = wsp.open('./cpu-user.wsp', wsp.MMAP)
print w.archives[0].points()
