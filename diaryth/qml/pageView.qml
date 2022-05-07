import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1
import QtWebView 1.12


Item {

    id: pageItem


    Component.onCompleted:
    {
        webView.url = "https://www.google.com/"
    }


    ColumnLayout
    {
        x: 25
        y: 25
        spacing: 10

        RowLayout
        {
            spacing:  10

            RoundButton {
                text: "just"
            }
        }


        WebView {
            id: webView
        }


    }


    function keyboardEventSend(key, mode) {
        //Заглушка обработки клавиш
    }

}
