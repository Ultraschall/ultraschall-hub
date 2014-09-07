#!/usr/bin/env ruby -wU

###################################################################
# install Ultraschall to /Library/Extensions and start it
# requires admin permissions and will ask for your password
###################################################################

#require 'open3'
require 'fileutils'
require 'pathname'
#require 'rexml/document'
#include REXML

# This finds our current directory, to generate an absolute path for the require
libdir = "."
Dir.chdir libdir        # change to libdir so that requires work

@svn_root = ".."

puts "  Unloading and removing existing UltraschallHub.kext"
if File.exists?("/Library/Extensions/UltraschallHub.kext")
  puts "    first unload (will often fail, but will cause UltraschallHub's performAudioEngineStop to be called)"
  `sudo kextunload /Library/Extensions/UltraschallHub.kext`
  puts "    second unload (this one should work)"
  `sudo kextunload /Library/Extensions/UltraschallHub.kext`
  puts "    removing"
  puts `sudo rm -rf /Library/Extensions/UltraschallHub.kext`
end

puts "  Copying to /Library/Extensions and loading kext"
`sudo cp -rv ../Build/UltraschallHub.kext /Library/Extensions`
`sudo kextutil -t /Library/Extensions/UltraschallHub.kext`
`sudo kextload /Library/Extensions/UltraschallHub.kext`
`sudo touch /Library/Extensions`

puts "  Done."
puts ""
exit 0
