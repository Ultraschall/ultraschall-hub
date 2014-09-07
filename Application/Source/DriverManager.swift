import Foundation

public struct DriverManager {
    func load(driverPath: String) -> Bool {
        if let param = driverPath.cStringUsingEncoding(NSUTF8StringEncoding) {
            return DMLoadDriver(param)
        } else {
            return false
        }
    }

    func unload(driverPath: String) -> Bool {
        if let param = driverPath.cStringUsingEncoding(NSUTF8StringEncoding) {
            return DMUnloadDriver(param)
        } else {
            return false
        }
    }
    
    func isLoaded(bundleId: String) -> Bool {
        if let param = bundleId.cStringUsingEncoding(NSUTF8StringEncoding) {
            return (DMQueryDriverStatus(param, nil) == 1) ? true : false
        } else {
            return false;
        }
    }
    
    public func activate(driverPath: String) -> Bool {
        if isLoaded(driverPath) {
            unload(driverPath)
        }
        
        return load(driverPath)
    }
}

