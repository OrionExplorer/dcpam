#!/usr/bin/python3
import sys
import time

if __name__ == '__main__':
	time.sleep(5)
	sys.stdout.buffer.write( "1".encode('utf8') )
	sys.stdout.flush()
