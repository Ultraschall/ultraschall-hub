
import Foundation

//    let plist = "/Library/Extensions/Soundflower.kext/Contents/Info.plist"

// ["IOKitPersonalities"] -> Dictionary
//     ["PhantomAudioDriver"] -> Dictionary
//         ["AudioEngines"] -> Array


public struct AudioEngineManager {
    public init() {
        self.loadConfiguration("/Users/Heiko/Info.plist")
    }
    
    public mutating func addEngine(description: String, numChannels: Int) -> Bool {
        if self.engines_.indexForKey(description) == nil {
            self.engines_[description] = numChannels
            return true;
        }
        
        return false
    }
    
    public mutating func updateEngine(description: String, numChannels: Int) -> Bool {
        if self.engines_.indexForKey(description) != nil {
            self.engines_[description] = numChannels
            return true;
        }
        
        return false
    }
    
    public mutating func removeEngine(description: String) -> Bool {
        if self.engines_.indexForKey(description) != nil {
            self.engines_.removeValueForKey(description)
            return true;
        }
        
        return false
    }
    
    public mutating func loadConfiguration(path: String) -> Bool {
        self.engines_.removeAll(keepCapacity: false)
        
        if let configuration = NSDictionary(contentsOfFile: path) as? [String: AnyObject] {
            if let personalities = configuration["IOKitPersonalities"]! as? [String : AnyObject] {
                if let phantom = personalities["PhantomAudioDriver"]! as? [String : AnyObject] {
                    if let engines = phantom["AudioEngines"]! as? [AnyObject] {
                        for engine in engines {
                            if let description = engine["Description"]! as? String {
                                if let formats = engine["Formats"]! as? [AnyObject] {
                                    if let numChannels = formats[0]["IOAudioStreamNumChannels"] as? Int {
                                        self.engines_[description] = numChannels
                                    }
                                }
                            }
                        }
                        
                        return self.engines_.count > 0
                    }
                }
            }
        }
        
        return false
    }
    
    public func saveConfiguration(path: String) -> Bool {
        return false
    }

    private var engines_: [String : Int] = [String : Int]()
    
    var engines: [AudioEngine] {
        get {
            var enginesAsArray: [AudioEngine] = [AudioEngine]()
            for (description, numChannels) in self.engines_ {
                enginesAsArray.append(AudioEngine(description: description, numChannels: numChannels));
            }
            
            return enginesAsArray
        }
    }
}

