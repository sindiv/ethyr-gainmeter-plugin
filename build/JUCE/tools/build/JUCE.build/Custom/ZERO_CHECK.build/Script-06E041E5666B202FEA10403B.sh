#!/bin/sh
set -e
if test "$CONFIGURATION" = "Custom"; then :
  cd /Users/divijsingh/repos/gainmeter-plugin/build/JUCE/tools
  make -f /Users/divijsingh/repos/gainmeter-plugin/build/JUCE/tools/CMakeScripts/ReRunCMake.make
fi

