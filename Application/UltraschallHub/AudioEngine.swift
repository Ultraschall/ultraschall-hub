//
//  AudioEngine.swift
//  UltraschallHub
//
//  Created by Daniel Lindenfelser on 04.10.14.
//

import Cocoa

class AudioEngine : NSObject, NSCoding {
    var engineDescription: String
    var engineChannels: Int
    var engineIdentifier: String
    
    override init () {
        self.engineIdentifier = NSUUID().UUIDString
        self.engineDescription = "Ultraschall (Generic)"
        self.engineChannels = 2
    }
    
    init(description: String, channels: Int) {
        self.engineIdentifier = NSUUID().UUIDString
        self.engineDescription = description
        self.engineChannels = channels
    }
    
    init(description: String, channels: Int, identifier: String) {
        self.engineIdentifier = identifier
        self.engineDescription = description
        self.engineChannels = channels
    }
    
    class func fromTemplate() -> AudioEngine? {
        if let file = NSBundle.mainBundle().pathForResource("AudioEngineTemplate", ofType: "plist") {
            if let template = NSDictionary(contentsOfFile: file) {
                return fromDictionary(template)
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
    
    func asDictionary() -> NSDictionary? {
        if let file = NSBundle.mainBundle().pathForResource("AudioEngineTemplate", ofType: "plist") {
            if let template = NSMutableDictionary(contentsOfFile: file) {
                template.setObject(engineDescription, forKey: "Description")
                template.setObject(engineIdentifier, forKey: "Identifier")
                if let formats = template.objectForKey("Formats") as? NSArray {
                    if let element = formats[0] as? NSMutableDictionary {
                        element.setObject(engineChannels, forKey: "IOAudioStreamNumChannels")
                        return template as NSDictionary?
                    }
                }
            }
        }
        return nil
    }
    
    required init(coder aDecoder: NSCoder) {
        self.engineIdentifier = aDecoder.decodeObjectForKey("engineIdentifier") as! String
        self.engineDescription  = aDecoder.decodeObjectForKey("engineDescription") as! String
        self.engineChannels = aDecoder.decodeObjectForKey("engineChannels") as! Int
    }
    
    func encodeWithCoder(aCoder: NSCoder) {
        aCoder.encodeObject(engineDescription, forKey: "engineDescription")
        aCoder.encodeObject(engineChannels, forKey: "engineChannels")
        aCoder.encodeObject(engineIdentifier, forKey: "engineIdentifier")
    }
}
