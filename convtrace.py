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
print "[Ciphertexts]"
print "ciphertext_type=u"
print "ciphertext=ciphertext %d %d" %(textout.shape[0], textout.shape[1])
print """

[General]

# The number of threads used in the computation
threads=4

# The order of the attack.
# 1: first-order (default)
# 2: second-order, if window=1 corresponds to second order moment
# n (>2): n-th order standardized moments 
order=1

# The return type of the correlation.
return_type=double

# The size of the window when computing 2-order CPA
# Warning, window starts indexing at the position of the first index.
# e.g. i=23, window=4 => 23, 24, 25, 26
window=3

# The algorithm to attack can be
# AES
# DES
algorithm=AES

# The position to attack, must be specified as a path to the file containing 
# the corresponding lookup table. Warning, there is a specific trigger for DES
position=LUT/AES_AFTER_SBOX

# The switch specific to DES, specifying the layout of the lookup table
# WARNING: If attacking multiple SBOXES, the layout must be the same 
# among them
# Can be either of
# DES_8_64 : 8 arrays of 64 values
# DES_8_64_ROUND : same as DES_8_64 but target is round output, after
#	recombination of SBOX output and half left state
# DES_32_16 : 32 arrays of 16 values, where the offset is selected with 
#	the first and last bits of the internal 6-bit values, and the
#	lookup is done on the 4 middle bits
# DES_4_BITS : Ignores lookup table and just compute the correlation 
#	before the SBOX on the 4 middle bits. However, one must still 
#	specify a file in order for it to work
# DES_6_BITS : Ignores lookup table and just compute the correlation 
#	before the SBOX on the 6 bits. However, one must still 
#	specify a file in order for it to work
#des_switch=DES_8_64

# The round to attack. Only the first (round=0) is supported for the moment.
round=0

# The number of the key byte to attack. Can be either an integer or "all"
bytenum=all
#bytenum=0

# The bit number in the byte to attack. 
# Options: none, all, <int>
# Where int is 0, ..., 7 for AES and 0, ..., 3 for DES
# Default (if unspecified) is none
bitnum=all

# The correct key if we know it. If unspecified, the program outputs
# the highest correlation for all keys. If speficied, only outputs the
# rank of the known key. This behavious is similar to inspector.
# Might also be used later for known key attacks. 
# Can be specified:
#   as a single key byte (in decimal or in hexadecimal with 0x prefix),
#   as the whole key in hex (0x8f f9 8b 4a 7a a3 5b 69 30 1c b2 a7 91 07 6b 7e)
# WARNING: Specifying a single key byte does not work with DES, as we need
#	the complete key to generate the round keys.
#correct_key=100
#correct_key=0x8f f9 8b 4a 7a a3 5b 69 30 1c b2 a7 91 07 6b 7e
#correct_key=0x2B 7E 15 16 28 AE D2 A6 AB F7 15 88 09 CF 4F 3C

# The memory available. Used with attack=50 only. Possible suffixes:
# G: Giga
# M: Mega
memory=4G

# The number of global correlation we keep track of.
top=20

# The separator for printing, if not specified, normal prints with many details
# if something is specified, prints in the following format:
# byte_number "sep" key_byte "sep" time "sep" key "sep" correlation
# But this only works when the correct key specified
#separator=, 
"""


