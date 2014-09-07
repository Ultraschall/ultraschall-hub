#!/usr/bin/env ruby -wU

###################################################################
# uninstall Ultraschall
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

puts "  Unloading and removing existing Soundflower.kext from /Library/Extensions"
if File.exists?("/Library/Extensions/Soundflower.kext")
  puts "    first unload (will often fail, but will cause Soundflower's performAudioEngineStop to be called)"
  `sudo kextunload /Library/Extensions/Soundflower.kext`
  puts "    second unload (this one should work)"
  `sudo kextunload /Library/Extensions/Soundflower.kext`
  puts "    removing"
  puts `sudo rm -rf /Library/Extensions/Soundflower.kext`
end

puts "  Unloading and removing existing Soundflower.kext from /System/Library/Extensions"
if File.exists?("/System/Library/Extensions/Soundflower.kext")
  puts "    first unload (will often fail, but will cause Soundflower's performAudioEngineStop to be called)"
  `sudo kextunload /System/Library/Extensions/Soundflower.kext`
  puts "    second unload (this one should work)"
  `sudo kextunload /System/Library/Extensions/Soundflower.kext`
  puts "    removing"
  puts `sudo rm -rf /System/Library/Extensions/Soundflower.kext`
end

puts "  Unloading and removing existing UltraschallHub.kext"
if File.exists?("/Library/Extensions/UltraschallHub.kext")
  puts "    first unload (will often fail, but will cause UltraschallHub's performAudioEngineStop to be called)"
  `sudo kextunload /Library/Extensions/UltraschallHub.kext`
  puts "    second unload (this one should work)"
  `sudo kextunload /Library/Extensions/UltraschallHub.kext`
  puts "    removing"
  puts `sudo rm -rf /Library/Extensions/UltraschallHub.kext`
end

puts "  Done."
puts ""
exit 0
