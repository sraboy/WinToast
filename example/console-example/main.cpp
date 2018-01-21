#include "wintoastlib.h"

using namespace WinToastLib;

class CustomHandler : public IWinToastHandler {
public:
    void toastActivated() const {
        std::wcout << L"The user clicked in this toast" << std::endl;
        exit(0);
    }

    void toastActivated(int actionIndex) const {
        std::wcout << L"The user clicked on action #" << actionIndex << std::endl;
        exit(16 + actionIndex);
    }

    void toastDismissed(WinToastDismissalReason state) const {
        switch (state) {
        case UserCanceled:
            std::wcout << L"The user dismissed this toast" << std::endl;
            exit(1);
            break;
        case TimedOut:
            std::wcout << L"The toast has timed out" << std::endl;
            exit(2);
            break;
        case ApplicationHidden:
            std::wcout << L"The application hid the toast using ToastNotifier.hide()" << std::endl;
            exit(3);
            break;
        default:
            std::wcout << L"Toast not activated" << std::endl;
            exit(4);
            break;
        }
    }

    void toastFailed() const {
        std::wcout << L"Error showing current toast" << std::endl;
        exit(5);
    }
};


enum Results {
	ToastClicked,					// user clicked on the toast
	ToastDismissed,					// user dismissed the toast
	ToastTimeOut,					// toast timed out
	ToastHided,						// application hid the toast
	ToastNotActivated,				// toast was not activated
	ToastFailed,					// toast failed
	SystemNotSupported,				// system does not support toasts
	UnhandledOption,				// unhandled option
	MultipleTextNotSupported,		// multiple texts were provided
	InitializationFailure,			// toast notification manager initialization failure
	ToastNotLaunched				// toast could not be launched
};


#define COMMAND_ACTION		L"--action"
#define COMMAND_AUMI		L"--aumi"
#define COMMAND_APPNAME		L"--appname"
#define COMMAND_APPID		L"--appid"
#define COMMAND_EXPIREMS	L"--expirems"
#define COMMAND_TEXT		L"--text"
#define COMMAND_HELP		L"--help"
#define COMMAND_IMAGE		L"--image"
#define COMMAND_SHORTCUT	L"--only-create-shortcut"
#define COMMAND_CUSTOMXML   L"--custom-xml"

void print_help() {
	std::wcout << "WinToast Console Example [OPTIONS]" << std::endl;
	std::wcout << "\t" << COMMAND_ACTION << L" : Set the actions in buttons" << std::endl;
	std::wcout << "\t" << COMMAND_AUMI << L" : Set the App User Model Id" << std::endl;
	std::wcout << "\t" << COMMAND_APPNAME << L" : Set the default appname" << std::endl;
	std::wcout << "\t" << COMMAND_APPID << L" : Set the App Id" << std::endl;
	std::wcout << "\t" << COMMAND_EXPIREMS << L" : Set the default expiration time" << std::endl;
	std::wcout << "\t" << COMMAND_TEXT << L" : Set the text for the notifications" << std::endl;
	std::wcout << "\t" << COMMAND_IMAGE << L" : set the image path" << std::endl;
	std::wcout << "\t" << COMMAND_SHORTCUT << L" : create the shortcut for the app" << std::endl;
	std::wcout << "\t" << COMMAND_HELP << L" : Print the help description" << std::endl;
    std::wcout << "\t" << COMMAND_CUSTOMXML << L" : Set custom XML" << std::endl;
}


int wmain(int argc, LPWSTR *argv)
{
	if (argc == 1) {
		print_help();
		return 0;
	}

    if (!WinToast::isCompatible()) {
        std::wcerr << L"Error, your system in not supported!" << std::endl;
        return Results::SystemNotSupported;
    }

    LPWSTR appName = L"Console WinToast Example", appUserModelID = L"WinToast Console Example", text = NULL, imagePath = NULL, customXmlTemplate = NULL;
    std::vector<std::wstring> actions;
    INT64 expiration = 0;

    bool onlyCreateShortcut = false;

    int i;
	for (i = 1; i < argc; i++)
        if (!wcscmp(COMMAND_IMAGE, argv[i]))
            imagePath = argv[++i];
        else if (!wcscmp(COMMAND_ACTION, argv[i]))
            actions.push_back(argv[++i]);
        else if (!wcscmp(COMMAND_EXPIREMS, argv[i]))
            expiration = wcstol(argv[++i], NULL, 10);
        else if (!wcscmp(COMMAND_APPNAME, argv[i]))
            appName = argv[++i];
        else if (!wcscmp(COMMAND_AUMI, argv[i]) || !wcscmp(COMMAND_APPID, argv[i]))
            appUserModelID = argv[++i];
		else if (!wcscmp(COMMAND_TEXT, argv[i]))
			text = argv[++i];
		else if (!wcscmp(COMMAND_SHORTCUT, argv[i]))
			onlyCreateShortcut = true;
        else if (!wcscmp(COMMAND_CUSTOMXML, argv[i]))
            customXmlTemplate = argv[++i];
		else if (!wcscmp(COMMAND_HELP, argv[i])) {
			print_help();
			return 0;
		} else {
            std::wcerr << L"Option not recognized: " << argv[i]  << std::endl;
			return Results::UnhandledOption;
        }

    WinToast::instance()->setAppName(appName);
    WinToast::instance()->setAppUserModelId(appUserModelID);

    if (onlyCreateShortcut) {
        if (imagePath || text || actions.size() > 0 || expiration || customXmlTemplate) {
            std::wcerr << L"--only-create-shortcut does not accept images/text/actions/expiration/custom XML" << std::endl;
            return 9;
        }
        enum WinToast::ShortcutResult result = WinToast::instance()->createShortcut();
        return result ? 16 + result : 0;
    }

    if (!text)
        text = L"Hello, world!";

    if (!WinToast::instance()->initialize()) {
        std::wcerr << L"Error, your system in not compatible!" << std::endl;
        return Results::InitializationFailure;
    }

    bool withImage = (imagePath != NULL);
	WinToastTemplate templ( withImage ? WinToastTemplate::CustomXmlImageAndText : WinToastTemplate::CustomXmlText);
	templ.setTextField(text, WinToastTemplate::FirstLine);
    
	for (auto const &action : actions)
        templ.addAction(action);
    if (expiration)
        templ.setExpiration(expiration);
    if (withImage)
        templ.setImagePath(imagePath);
    /*
    std::wstring pre_xml(L"<toast duration='short'>"
                         L" <audio silent='true'/>"
                         L" <visual>"
                         L" <binding template='ToastGeneric'>");

    std::wstring image(L" <image placement='appLogoOverride' hint-crop='circle' src=''/>");
    
    std::wstring content(L" <text hint-maxLines='1'>Encryption Succesful!</text>"
                         L" <text placement='attribution'>via SMS</text>");

    std::wstring post_xml(L" </binding>"
                          L" </visual>"
                          L"</toast>");
    std::wstring allxml(pre_xml + image + content + post_xml);*/
    if (WinToast::instance()->showToast(templ, new CustomHandler(), customXmlTemplate) < 0) {
        std::wcerr << L"Could not launch your toast notification!";
		return Results::ToastFailed;
    }

    // Give the handler a chance for 15 seconds (or the expiration plus 1 second)
    Sleep(expiration ? (DWORD)expiration + 1000 : 15000);

    exit(2);
}
