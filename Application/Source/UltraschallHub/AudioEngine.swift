import Foundation

public struct AudioEngine {
    public init(description: String, numChannels: Int) {
        self.description = description;
        self.numChannels = numChannels;
    }
    
    public static func fromTemplate() -> AudioEngine? {
        if let template = NSDictionary(contentsOfFile: "AudioEngineTemplate.plist") as? [String : AnyObject] {
            if let description = template["Description"]! as? String {
                if let formats = template["Formats"]! as? [AnyObject] {
                    if let numChannels = formats[0]["IOAudioStreamNumChannels"] as? Int {
                        return AudioEngine(description: description, numChannels: numChannels)
                    }
                }
            }
        }

        return nil
    }
    
    var description: String = "<Unknown>"
    var numChannels = 2
}
