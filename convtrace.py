#!/usr/bin/python
import sys
from sys import argv
import numpy as np
import struct

if len(argv) != 4:
    print('usage: %s <traces> <pts> <cts>')
    sys.exit(1)

traces = np.load(argv[1])
textin = np.load(argv[2])
textout = np.load(argv[3])

f = open('tracefile', 'wb')

for t in traces.flatten():
    f.write(struct.pack('f', t))
    
f.close()

f = open('plaintext', 'wb')
for t in textin.flatten():
    f.write(struct.pack('B', t))
f.close()

f = open('ciphertext', 'wb')
for t in textout.flatten():
    f.write(struct.pack('B', t))
f.close()



print "Add following to CONFIG:\n"
print "[Traces]"
print "files=1"
print "trace_type=f"
print "transpose=true"
print "index=0"
print "nsamples=%d"%traces.shape[1]
print "trace=tracefile %d %d"%(traces.shape[0], traces.shape[1])
print ""
print "[Guesses]"
print "files=1"
print "guess_type=u"
print "transpose=true"
print "guess=plaintext %d %d"%(textin.shape[0], textin.shape[1])
print ""
print "[Ciphertext]"
print "ciphertext_type=u"
print "ciphertext=ciphertext %d %d" %(textout.shape[0], textout.shape[1])


