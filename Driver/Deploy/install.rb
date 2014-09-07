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

puts "  Unloading and removing existing Ultraschall.kext"
if File.exists?("/Library/Extensions/Ultraschall.kext")
  puts "    first unload (will often fail, but will cause Ultraschall's performAudioEngineStop to be called)"
  `sudo kextunload /Library/Extensions/Ultraschall.kext`
  puts "    second unload (this one should work)"
  `sudo kextunload /Library/Extensions/Ultraschall.kext`
  puts "    removing"
  puts `sudo rm -rf /Library/Extensions/Ultraschall.kext`
end

puts "  Copying to /Library/Extensions and loading kext"
`sudo cp -rv Ultraschall.kext /Library/Extensions`
`sudo kextutil -t /Library/Extensions/Ultraschall.kext`
`sudo kextload /Library/Extensions/Ultraschall.kext`
`sudo touch /Library/Extensions`

puts "  Done."
puts ""
exit 0
