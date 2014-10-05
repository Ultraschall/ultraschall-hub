import Cocoa

class AudioEngineManager {
    var audioEngines = NSMutableDictionary()
    
    init() {
        loadDriverConfiguration()
    }
    
    // MARK: - Engine Control
    
    func engineAtIndex(index: Int) -> AudioEngine? {
        var array = audioEngines.keysSortedByValueUsingComparator { (obj1, obj2) -> NSComparisonResult in
            var engine1 = obj1 as AudioEngine
            var engine2 = obj2 as AudioEngine
            return engine1.engineDescription.caseInsensitiveCompare(engine2.engineDescription)
        }
        return audioEngines.objectForKey(array[index]) as AudioEngine?
    }
    
    func addEngine(description: String, numChannels: Int) -> Bool {
        var engine = AudioEngine(description: description, channels: numChannels)
        audioEngines[engine.engineIdentifier] = engine
        return false
    }

    func addEngine(engine: AudioEngine!) -> Bool {
        if audioEngines.objectForKey(engine.engineIdentifier) == nil {
            audioEngines[engine.engineIdentifier] = engine
            return true
        }
        return false
    }
    
    func updateEngine(description: String, numChannels: Int, identifier: String!) -> Bool {
        if audioEngines.objectForKey(identifier) != nil {
            let value = audioEngines.objectForKey(identifier) as AudioEngine!
            value.engineDescription = description
            value.engineChannels = numChannels
            return true;
        }
        
        return false
    }
    
    func updateEngine(engine: AudioEngine!) -> Bool {
        if audioEngines.objectForKey(engine.engineIdentifier) != nil {
            let value = audioEngines.objectForKey(engine.engineIdentifier) as AudioEngine!
            value.engineDescription = engine.engineDescription
            value.engineChannels = engine.engineChannels
            return true;
        }
        
        return false
    }
    
    func removeEngine(identifier: String) -> Bool {
        if audioEngines.objectForKey(identifier) != nil {
            audioEngines.removeObjectForKey(identifier)
            return true;
        }
        
        return false
    }
    
    func removeEngine(engine: AudioEngine!) -> Bool {
        if audioEngines.objectForKey(engine.engineIdentifier) != nil {
            audioEngines.removeObjectForKey(engine.engineIdentifier)
            return true;
        }
        
        return false
    }
    
    func getEngines(baseDictionary: NSDictionary) -> NSArray? {
        if let configuration = baseDictionary as? [String: AnyObject] {
            if let personalities = configuration["IOKitPersonalities"]! as? [String : AnyObject] {
                if let phantom = personalities["PhantomAudioDriver"]! as? [String : AnyObject] {
                    if let engines = phantom["AudioEngines"]! as? NSArray {
                        return engines
                    }
                }
            }
        }
        return nil
    }
    
    // MARK: - Driver Configuration
    
    func pathForTemporaryFile() -> String {
        var uuid = CFUUIDCreateString(nil, CFUUIDCreate(nil)) as String
        return NSTemporaryDirectory().stringByAppendingPathComponent(uuid)
    }
    
    func loadDriverConfiguration() {
        loadConfiguration("/Library/Extensions/UltraschallHub.kext/Contents/Info.plist")
    }
    
    func loadConfiguration(path: String) -> Bool {
        audioEngines.removeAllObjects()
        
        var configuration = NSDictionary(contentsOfFile: path)
        if configuration == nil {
            return false
        }
        
        var engines = getEngines(configuration!)
        for engine in engines! {
            var newEngine = AudioEngine.fromDictionary(engine as NSDictionary)
            if newEngine != nil {
                audioEngines[newEngine!.engineIdentifier] = newEngine
            }
        }
        
        return false
    }

    func saveConfiguration() -> Bool {
        return saveConfiguration(pathForTemporaryFile())
    }
    
    func saveConfiguration(path: String) -> Bool {
        var configuration = NSMutableDictionary(contentsOfFile: "/Library/Extensions/UltraschallHub.kext/Contents/Info.plist")
        if configuration == nil {
            return false
        }
        
        if let personalities = configuration!["IOKitPersonalities"]! as? NSDictionary {
            if let phantom = personalities["PhantomAudioDriver"]! as? NSMutableDictionary {
                var engines = phantom["AudioEngines"] as NSMutableArray
                engines.removeAllObjects()
                for kv in audioEngines {
                    var engine = kv.value as AudioEngine
                    engines.insertObject(engine.getDictionary()!, atIndex: 0);
                }
                return configuration!.writeToFile(path, atomically: true)
            }
        }
        return false
    }
    
    // MARK: - Presets
    
    func newPreset() {
        audioEngines.removeAllObjects()
    }
    
    func loadPreset(path: String) -> Bool {
        var preset = NSKeyedUnarchiver.unarchiveObjectWithFile(path) as NSMutableDictionary?
        if (preset != nil) {    
            audioEngines.removeAllObjects()
            audioEngines.setDictionary(preset!)
            return true
        }
        return false
    }
    
    func savePreset(path: String) -> Bool {
        var preset = audioEngines as NSDictionary
        return NSKeyedArchiver.archiveRootObject(preset, toFile: path)
    }
}

