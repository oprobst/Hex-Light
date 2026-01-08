#!/bin/bash
# Build-Script mit Logging

LOGFILE="build.log"
TIMESTAMP=$(date "+%Y-%m-%d %H:%M:%S")

echo "=====================================" >> $LOGFILE
echo "Build started: $TIMESTAMP" >> $LOGFILE
echo "=====================================" >> $LOGFILE

# PlatformIO Build ausführen und in Log schreiben
~/.platformio/penv/bin/pio run 2>&1 | tee -a $LOGFILE

echo "" >> $LOGFILE
echo "Build finished: $(date "+%Y-%m-%d %H:%M:%S")" >> $LOGFILE
echo "" >> $LOGFILE
