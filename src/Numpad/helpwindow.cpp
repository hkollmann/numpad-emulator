#include "helpwindow.h"
#include <QTextBrowser>
#include <QVBoxLayout>
#include <Windows.h>
#include <QApplication>
#include <QDesktopWidget>


HelpWindow::HelpWindow(QWidget *parent)
    : QWidget(parent, Qt::WindowTitleHint | Qt::WindowStaysOnTopHint)
{
    setWindowTitle("Help");
    textBrowser = new QTextBrowser;
    QFont font = textBrowser->font();
    font.setPixelSize(14);
    textBrowser->setFont(font);
    int minSide;
    if (QApplication::desktop()->height() < QApplication::desktop()->width())
    {
        minSide = QApplication::desktop()->height();
    }
    else
    {
        minSide = QApplication::desktop()->width();
    }
    setMinimumHeight(minSide / 2);
    setMinimumWidth(minSide / 2);

    textBrowser->setHtml(
                "<html>"
                "<div id=\"start\">"
                  "<h3>Hide/show the keyboard</h3>"
                "</div>"
                "<p>"
                "Since version 1.5 you can minimize/restore the keyboard. It is "
                "also possible to use a special key to hide/show the keyboard (F9 "
                "by default). In the Settings you can assign another key (F1-F12, "
                "0-9, A-Z). "
                "</p>"
                "<p>"
                "Numpad is always available in the system tray (bottom right). "
                "</p>"
                "<h3>Add/remove buttons</h3>"
                "<p>"
                "Numpad is actually a full configurable keyboard. Your keyboard "
                "can consist of one button or one hundred buttons. In any combination, "
                "anywhere. You can even add buttons with whole words. For example, "
                "button with text \"Hello, Numpad\". When you press this button, "
                "\"Hello, Numpad\" will be printed. "
                "</p>"
                "<div id=\"ctrlShiftAlt\">"
                "<p>"
                "<b>IMPORTANT: Ctrl, Shift, Alt</b> work with \"A key\"-\"Z key\", "
                "\"0 key\"-\"9 key\" and do not work with \"a\"-\"z\", \"A\"-\"Z\", "
                "\"0\"-\"9\". "
                "</p>"
                "</div>"
                "<p>"
                "<b>Alt codes</b> work with \"0\"-\"9\" and do not work with \"0 key\"-\"9 key\". "
                "Alt codes are special characters. Do not confuse with the usual use of "
                "Alt key. "
                "</p>"
                "<p>"
                "What is the difference between, for example, <b>\"A\"</b> and <b>\"A key\"</b>? "
                "\"A\" is always \"A\". \"A key\" can be \"A\" or \"a\" or something else, "
                "depending on your language. "
                "</p>"
                "<p>"
                "It is also possible to load a predefined configuration. Press "
                "\"Load other configuration\" in Settings and select a configuration file. "
                "In folder \"Examples\" you can find some configurations. "
                "</p>"
                "<h3>Moving your buttons configuration to another computer</h3>"
                "<p>"
                "Go to Settings, press \"Open configuration files folder\", go to "
                "\"Current\" folder and copy file NumpadConfig.txt to another computer to "
                "any place. On another computer go to Settings, press \"Load other "
                "configuration\" and select this file. This file is no longer needed. "
                "</p>"
                "<h3>Special symbols (Alt codes)</h3>"
                "<p>"
                "In order to use special symbols (Alt codes) you first need to add Alt button. "
                "Select item \"Add/Remove buttons\" to use a visual configurator. "
                "Or you can load a predefined configuration with Alt button. "
                "To do this go to Settings, select \"Load other configuration\" "
                "and check folder \"Examples\". "
                "</p>"
                "<p>"
                "Now press Alt button, press any number, press Alt button again "
                "and you will get a special symbol. There are many Alt codes "
                "that you can find on the Internet. "
                "</p>"
                "<p>"
                "Alt codes work with \"0\"-\"9\" and do not work with \"0 key\"-\"9 key\". "
                "</p>"
                "<div id=\"createAltCodeBtn\">"
                  "<h3>How to create a button with special symbol (Alt code)</h3>"
                  "<p>"
                  "For example, you want to create a button with symbol &#x00A9;. "
                  "Go to the Internet and find \"Alt codes\". You need a table with "
                  "alt codes. Find the code corresponding to the &#x00A9;. It's 0169. "
                  "Enter these numbers in the text box \"Alt code\" (0169, not 169). "
                  "(<b>Note:</b> Some alt codes vary depending on your language. "
                  "Therefore, you need a alt codes table corresponding to your language.) "
                  "</p>"
                  "<p>"
                  "Now open the \"Character map\" on your windows-machine. Find the "
                  "symbol &#x00A9; and click on it. At the bottom left you will see "
                  "the text: \"U+00A9: Copyright Sign \".  <b>00A9</b> is what you "
                  "have to enter in the text box \"Unicode\". (The letter A must be "
                  "English.) Press \"Create special symbol\". "
                  "</p>"
                "</div>"
                "</html>"
                );
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(textBrowser);
    setLayout(layout);

    HWND hwnd = winId();
    LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(hwnd, GWL_EXSTYLE, styles | WS_EX_NOACTIVATE);
}


void HelpWindow::moveToAnchor(const QString &anchor)
{
    textBrowser->scrollToAnchor(anchor);
}


