import Cocoa

class AudioEngine : NSObject, NSCoding {
    var engineDescription: String
    var engineChannels: Int
    var engineIdentifier: String
    
    override init () {
        self.engineIdentifier = CFUUIDCreateString(nil, CFUUIDCreate(nil))
        self.engineDescription = "Ultraschall (Generic)"
        self.engineChannels = 2
    }
    
    required init(coder aDecoder: NSCoder) {
        self.engineIdentifier = aDecoder.decodeObjectForKey("engineIdentifier") as String
        self.engineDescription  = aDecoder.decodeObjectForKey("engineDescription") as String
        self.engineChannels = aDecoder.decodeObjectForKey("engineChannels") as Int
    }
    
    func encodeWithCoder(aCoder: NSCoder) {
        aCoder.encodeObject(engineDescription, forKey: "engineDescription")
        aCoder.encodeObject(engineChannels, forKey: "engineChannels")
        aCoder.encodeObject(engineIdentifier, forKey: "engineIdentifier")
    }
    
    init(description: String, channels: Int) {
        self.engineIdentifier = CFUUIDCreateString(nil, CFUUIDCreate(nil))
        self.engineDescription = description
        self.engineChannels = channels
    }
    
    init(description: String, channels: Int, identifier: String) {
        self.engineIdentifier = identifier
        self.engineDescription = description
        self.engineChannels = channels
    }
    
    class func fromTemplate() -> AudioEngine? {
        if let template = NSDictionary(contentsOfFile: "AudioEngineTemplate.plist") as? [String : AnyObject] {
            if let description = template["Description"]! as? String {
                if let formats = template["Formats"]! as? [AnyObject] {
                    if let numChannels = formats[0]["IOAudioStreamNumChannels"] as? Int {
                        return AudioEngine(description: description, channels: numChannels)
                    }
                }
            }
        }

        return nil
    }

    class func fromDictionary(dictionary: NSDictionary!) -> AudioEngine? {
        if let description = dictionary["Description"]! as? String {
            if let formats = dictionary["Formats"]! as? [AnyObject] {
                if let numChannels = formats[0]["IOAudioStreamNumChannels"] as? Int {
                    if let identifier = dictionary["Identifier"] as? String {
                        if identifier != "" {
                            return AudioEngine(description: description, channels: numChannels, identifier: identifier)
                        }
                    }
                    return AudioEngine(description: description, channels: numChannels)
                }
            }
        }

        return nil
    }
    
    func getDictionary() -> NSDictionary? {
        var file = UltraschallHub.preferencePaneBundle().pathForResource("AudioEngineTemplate", ofType: "plist")
        var template = NSMutableDictionary(contentsOfFile: file!)
        template?.setObject(engineDescription, forKey: "Description")
        template?.setObject(engineIdentifier, forKey: "Identifier")
        if let formats = template!["Formats"]! as? NSMutableDictionary {
            template?.setObject(engineChannels, forKey: "IOAudioStreamNumChannels")
        }
        return template as NSDictionary?
    }
}
