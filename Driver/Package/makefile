all: package

Components.plist:
	pkgbuild --analyze --root ../Build Components.plist
	
Uninstall.sh:
	mkdir -p Build
	echo "#!/bin/sh" > Build/Uninstall.sh
	echo sudo kextunload /Library/Extensions/UltraschallHub.kext >> Build/Uninstall.sh
	echo sudo kextunload /Library/Extensions/UltraschallHub.kext >> Build/Uninstall.sh
	echo sudo rm -rf /Library/Extensions/UltraschallHub.kext >> Build/Uninstall.sh
	chmod +x Build/Uninstall.sh

preinstall:
	echo "#!/bin/sh" > preinstall
	echo kextunload /Library/Extensions/UltraschallHub.kext >> preinstall
	echo kextunload /Library/Extensions/UltraschallHub.kext >> preinstall
	echo rm -rf /Library/Extensions/UltraschallHub.kext >> preinstall
	chmod +x preinstall
	
postinstall:
	echo "#!/bin/sh" > postinstall
	echo touch /Library/Extensions >> postinstall
	echo kextload /Library/Extensions/UltraschallHub.kext >> postinstall
	chmod +x postinstall
	
package: Components.plist
	mkdir -p Build
	make preinstall postinstall
	pkgbuild --identifier fm.ultraschall.pkg.UltraschallDriver --version 1.0beta1 --root ../Build --component-plist Components.plist --install-location /Library/Extensions --scripts ./ Build/UltraschallDriver-1.2.pkg
	make Uninstall.sh
	
image: package
	hdiutil create -size 1m -srcfolder Build -fs HFS+ -volname UltraschallDriver-1.2 Build/UltraschallDriver-1.2.dmg
	
clean:
	rm -f Components.plist
	rm -f preinstall
	rm -f postinstall
	
cleanall:
	make clean
	rm -rf Build
	