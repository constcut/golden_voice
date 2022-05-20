import QtQuick 2.12
import diaryth 1.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1


Item
{
    id: item

    FileDialog
    {
        id: fileDialog
        title: "Please choose json speech report"
        folder: shortcuts.desktop
        onAccepted: {
            var filename = fileDialog.fileUrls[0].substring(8)
            jsonReport.loadFromFile(filename)

            flick.contentWidth = jsonReport.getFullWidth()
            visualReport1.width = jsonReport.getFullWidth()
            visualReport2.width =  jsonReport.getFullWidth()
            chunkId.model = jsonReport.getChunksCount()

            visualReport1.setParent(jsonReport)
            visualReport2.setParent(jsonReport)

            visualReport1.addPraatField("Jitter (rap)", "red", 20)
            visualReport1.addPraatField("Number of pulses", "orange", 1.2)
            visualReport1.setType(VisualTypes.PraatInfo)
            visualReport2.setType(VisualTypes.Pitch)

            fileDialog.close()
        }
        onRejected: {
            fileDialog.close()
        }
        nameFilters: [ "JSON report (*.json)" ]
    }


    function requestFileDialog()
    {
        fileDialog.open()
    }

    RowLayout
    {
            y: 5

            Button {
                text: "Unselect"
                onClicked: {
                    jsonReport.removeAllSelections()
                }

                width: 80
                implicitWidth: 80
            }

            Button
            {
                text : "Info"

                width: 50
                implicitWidth: 50

                onClicked:
                {
                    var events = jsonReport.getSelectedEvents()

                    eventsRepeater.model = 0
                    eventsRepeater.model = events.length + 4 //names, chunk, full, full - chunk

                    var fieldsNames = jsonReport.getPraatFieldsNames();
                    var fullText = "type, "

                    for (var j = 0; j < fieldsNames.length; ++j)
                        fullText += fieldsNames[j] + ", "

                    eventsRepeater.itemAt(0).setText( fullText )

                    for (var i = 0; i < events.length; ++i)
                    {
                        var eventLine = events[i]
                        var eventIdx = eventLine[0]
                        var word = jsonReport.getWordByIdx(eventIdx)

                        fullText = word + " "

                        for (j = 1; j < eventLine.length; ++j)
                            fullText += eventLine[j].toString() + ", "

                        eventsRepeater.itemAt(i + 1).setText( fullText )
                    }

                    var chunkInfo = jsonReport.getChunkInfo(parseInt(chunkId.currentText))

                    fullText = "chunk "
                    for (j = 0; j < chunkInfo.length; ++j)
                        fullText += chunkInfo[j].toString() + ", "

                    eventsRepeater.itemAt( events.length + 1 ).setText( fullText )

                    var fullInfo = jsonReport.getFullInfo()

                    fullText = "full "
                    for (j = 0; j < fullInfo.length; ++j)
                        fullText += fullInfo[j].toString() + ", "

                    eventsRepeater.itemAt( events.length + 2 ).setText( fullText )

                    fullText = "diff "
                    for (j = 0; j < chunkInfo.length; ++j) {
                        fullText += (fullInfo[j] - chunkInfo[j]).toFixed(3).toString() + ", "
                    }

                    eventsRepeater.itemAt( events.length + 3 ).setText( fullText )

                    popup.open()
                }
            } //Button

            Button {
                text: "Select chunk"
                onClicked: {
                    jsonReport.selectChunk(parseInt(chunkId.currentText))
                }
            }

            ComboBox {
                id: chunkId

                width: 40
                implicitWidth: 40
            }


            ComboBox {
                id: firstReportType
                model: ["Pitch", "Intensity", "Praat", "PraatChunkDiff", "PraatFullDiff",
                        "Chunks", "Words", "ReportFields"]

                currentIndex: 2

                onCurrentTextChanged: {
                    visualReport1.setType(currentIndex + 1)
                }
            }

            Button {
                text: "Config"

                onClicked: {

                    if (firstReportType.currentIndex + 1 === VisualTypes.ReportFields)
                    {
                        configFieldsPopup.loadFromVisual(visualReport1)
                        configFieldsPopup.open()
                    }
                    else
                    {
                        configPopup.loadFromVisual(visualReport1)
                        configPopup.open()
                    }

                }
            }

            ComboBox {
                id: secondReportType
                model: ["Pitch", "Intensity", "Praat", "PraatChunkDiff", "PraatFullDiff",
                        "Chunks", "Words", "ReportFields"]

                onCurrentTextChanged: {
                    visualReport2.setType(currentIndex + 1)
                }
            }

            Button {
                text: "Config"

                onClicked:
                {
                    if (firstReportType.currentIndex + 1 === VisualTypes.ReportFields)
                    {
                        configFieldsPopup.loadFromVisual(visualReport2)
                        configFieldsPopup.open()
                    }
                    else
                    {
                        configPopup.loadFromVisual(visualReport2)
                        configPopup.open()
                    }
                }
            }

            Slider
            {
                from : 35
                to: item.height - 60
                stepSize: 5

                value: 190

                ToolTip.visible: hovered
                ToolTip.text: "Height " + value

                onMoved:
                {
                    //
                    //visualReport2.y = 5 + value
                    //visualReport2.height = item.height - value - 70
                    visualReport1.y = 5
                    visualReport1.height = value
                    visualReport2.y = value + 10
                    visualReport2.height = item.height - value - 70
                }
            }


            RoundButton {
                text: "+"
                onClicked:
                {
                    jsonReport.setZoom(jsonReport.getZoom() * 2)
                    flick.contentWidth = jsonReport.getFullWidth()
                    visualReport1.width = jsonReport.getFullWidth()
                    visualReport2.width =  jsonReport.getFullWidth()
                }
            }

            RoundButton {
                text: "-"
                onClicked: {
                    jsonReport.setZoom(jsonReport.getZoom() / 2)

                    flick.contentWidth = jsonReport.getFullWidth()
                    visualReport1.width = jsonReport.getFullWidth()
                    visualReport2.width =  jsonReport.getFullWidth()
                }
            }

            Button {
                text: "Open file"
                onClicked: fileDialog.open()
            }
    }


    ScrollView
    {
        id: scroll
        width: parent.width
        height: visualReport1.height + visualReport2.height

        y: 50

        Flickable
        {
            id: flick
            y: 0
            x: 0
            width: parent.width
            height: visualReport1.height + visualReport2.height
            contentWidth: 3000
            contentHeight:  parent.height
            property int pressedX : 0

            ScrollBar.horizontal: ScrollBar
            {
                height: 15
                active: true
                interactive: true
                policy: ScrollBar.AlwaysOn
            }

            VisualReport
            {
                id: visualReport1
                height:  190
                width: 3000
                y: 5

                MouseArea
                {
                    anchors.fill: parent
                    onClicked:{

                    }
                    onDoubleClicked: {
                        var idx = jsonReport.eventIdxOnClick(mouseX, mouseY)
                        jsonReport.selectEvent(idx)
                    }

                }
            } //VisualReport

            VisualReport
            {
                id: visualReport2
                height: 480
                width: 3000
                y: visualReport1.y + visualReport1.height + 5

                MouseArea
                {
                    anchors.fill: parent
                    onClicked:{

                    }
                    onDoubleClicked: {
                        var idx = jsonReport.eventIdxOnClick(mouseX, mouseY)
                        jsonReport.selectEvent(idx)
                    }

                }
            } //VisualReport

        } //Flickable

    } //ScrollView


    Popup
    {
        id: popup
        x: 50
        y: 50
        width: item.width - x * 2
        height: item.height - x * 2
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        Repeater
        {
            id: eventsRepeater

            RowLayout
            {
                x: 10
                y: index * 40

                Text {
                    id: eventText
                }

                function setText(text) {
                    eventText.text = text
                }
            }
        }

        Button
        {
            text: "Close"

            x: parent.width - width - 10
            y: parent.height - height - 5

            onClicked: popup.close()
        }
    }


    Popup
    {
        id: configPopup
        x: 50
        y: 50
        width: item.width - x * 2
        height: item.height - x * 2
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        DoubleValidator {
            id: realValidator
        }

        property var colorsNames: ["red", "green", "blue", "orange", "black", "darkRed", "darkGreen", "darkBlue",
                                   "cyan", "magenta", "yellow", "gray", "darkCyan", "darkMagenta",
                                   "darkYellow", "darkGray", "lightGray"]

        property var lastVisualReport: []
        property var storedFields: []

        function loadFromVisual(visualReport)
        {
            configPopup.lastVisualReport = visualReport

            var praatFields = visualReport.getPraatFields()

            configRepeater.model = 0
            configRepeater.model = praatFields.length

            for (var i = 0; i < praatFields.length; ++i)
            {
                var fieldLine = praatFields[i]
                configRepeater.itemAt(i).loadValues(fieldLine[0], fieldLine[1], fieldLine[2])
            }
        }


        function storeFields(skipIdx = -1)
        {
            var fieldsList = []

            for (var i = 0; i < configRepeater.model; ++i)
            {
                if (i === skipIdx)
                    continue

                var name = configRepeater.itemAt(i).getName()
                var color = configRepeater.itemAt(i).getColor()
                var yCoef = configRepeater.itemAt(i).getYCoef()
                var fieldLine = [name, color, yCoef]

                fieldsList.push(fieldLine)
            }

            configPopup.storedFields = fieldsList
        }

        function loadFields()
        {
            for (var i = 0; i < configPopup.storedFields.length; ++i)
            {
                var fieldLine = configPopup.storedFields[i]
                configRepeater.itemAt(i).loadValues(fieldLine[0], fieldLine[1], fieldLine[2])
            }
        }



        Repeater
        {
            model: 3
            id: configRepeater

            RowLayout
            {
                y: index * 60

                function loadValues(name, color, coef)
                {
                    var praatFieldsNames = jsonReport.getPraatFieldsNames()

                    for (var i = 0; i < praatFieldsNames.length; ++i)
                    {
                        if (praatFieldsNames[i] === name)
                            break;
                    }

                    praatFieldName.currentIndex = i

                    for (i = 0; i < configPopup.colorsNames.length; ++i)
                    {
                        if (configPopup.colorsNames[i] === color)
                            break;
                    }

                    fieldColor.currentIndex = i

                    fieldCoef.text = coef
                }

                function getName() {
                    return praatFieldName.currentText
                }

                function getColor() {
                    return fieldColor.currentText
                }

                function getYCoef() {
                    return fieldCoef.text
                }

                ComboBox
                {
                    id: praatFieldName
                    model: jsonReport.getPraatFieldsNames()

                    implicitWidth: 400
                }

                ComboBox {
                    id: fieldColor
                    model: configPopup.colorsNames
                }

                TextField {
                    id: fieldCoef
                    validator: realValidator
                    text: "1,0"
                }

                RoundButton {
                    text: "-"

                    onClicked: {
                        configPopup.storeFields(index)
                        loadStoredTimer.start() //Hot fix for ReferenceError: xyz is not defined
                        configRepeater.model = configRepeater.model - 1 //WE got issue here as our current object is child
                    }
                }

                RoundButton {
                    text: "+"

                    onClicked:
                    {
                        configPopup.storeFields()
                        loadStoredTimer.start() //Hot fix for ReferenceError: xyz is not defined
                        configRepeater.model = configRepeater.model + 1 //WE got issue here as our current object is child
                    }
                }
            }
        }

        Timer
        {
            id: loadStoredTimer
            interval: 50
            running: false
            repeat: false
            onTriggered: {
                configPopup.loadFields()
            }
        }

        Button
        {
            text: "Add new"

            x: parent.width/2 - width/2 - 5
            y: parent.height - height - 5

            onClicked:
            {
                configPopup.storeFields()
                loadStoredTimer.start()
                configRepeater.model += 1 //Как в HD версии
            }
        }


        Button
        {
            text: "Close"

            x: parent.width - width - 10
            y: parent.height - height - 5

            onClicked: configPopup.close()
        }

        Button
        {
            text: "Save"

            x: 10
            y: parent.height - height - 5

            onClicked:
            {
                configPopup.lastVisualReport.clearPraatFields() //find a way to use parametric values

                for (var i = 0; i < configRepeater.model; ++i)
                {

                    var name = configRepeater.itemAt(i).getName()
                    var color = configRepeater.itemAt(i).getColor()
                    var yCoef = parseFloat(configRepeater.itemAt(i).getYCoef())

                    configPopup.lastVisualReport.addPraatField(name, color, yCoef)
                }

                configPopup.lastVisualReport.setPraatType() //way to repaint

                configPopup.close()
            }
        }
    }




    Popup
    {
        id: configFieldsPopup
        x: 50
        y: 50
        width: item.width - x * 2
        height: item.height - x * 2
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        property var colorsNames: ["red", "green", "blue", "orange", "black", "darkRed", "darkGreen", "darkBlue",
                                   "cyan", "magenta", "yellow", "gray", "darkCyan", "darkMagenta",
                                   "darkYellow", "darkGray", "lightGray"]

        property var lastVisualReport: []
        property var storedFields: []

        function loadFromVisual(visualReport)
        {
            configFieldsPopup.lastVisualReport = visualReport
            configFieldsRepeater.model = 0

            var reportFields = visualReport.getReportFields()
            configFieldsRepeater.model = reportFields.length

            for (var i = 0; i < reportFields.length; ++i)
            {
                var fieldLine = reportFields[i]
                configFieldsRepeater.itemAt(i).loadValues(fieldLine[0], fieldLine[1], fieldLine[2])
            }
        }


        function storeFields(skipIdx = -1)
        {
            var fieldsList = []

            for (var i = 0; i < configFieldsRepeater.model; ++i)
            {
                if (i === skipIdx)
                    continue

                var name = configFieldsRepeater.itemAt(i).getName()
                var color = configFieldsRepeater.itemAt(i).getColor()
                var yCoef = configFieldsRepeater.itemAt(i).getYCoef()
                var fieldLine = [name, color, yCoef]

                fieldsList.push(fieldLine)
            }

            configFieldsPopup.storedFields = fieldsList
        }

        function loadFields()
        {
            for (var i = 0; i < configFieldsPopup.storedFields.length; ++i)
            {
                var fieldLine = configFieldsPopup.storedFields[i]
                configFieldsRepeater.itemAt(i).loadValues(fieldLine[0], fieldLine[1], fieldLine[2])
            }
        }



        Repeater
        {
            model: 3
            id: configFieldsRepeater

            RowLayout
            {
                y: index * 60

                function loadValues(name, color, coef)
                {
                    praatFieldName2.text = name
                    fieldCoef2.text = coef

                    for (var i = 0; i < configFieldsPopup.colorsNames.length; ++i)
                        if (configFieldsPopup.colorsNames[i] === color)
                            break;

                    fieldColor2.currentIndex = i
                }

                function getName() {
                    return praatFieldName2.text
                }

                function getColor() {
                    return fieldColor2.currentText
                }

                function getYCoef() {
                    return fieldCoef2.text
                }

                TextField
                {
                    id: praatFieldName2
                    implicitWidth: 400
                }

                ComboBox {
                    id: fieldColor2
                    model: configFieldsPopup.colorsNames
                }

                TextField {
                    id: fieldCoef2
                    validator: realValidator
                    text: "1,0"
                }

                RoundButton {
                    text: "-"

                    onClicked: {
                        configFieldsPopup.storeFields(index)
                        loadStoredFieldsTimer.start() //Hot fix for ReferenceError: xyz is not defined
                        configFieldsRepeater.model = configFieldsRepeater.model - 1 //WE got issue here as our current object is child
                    }
                }

                RoundButton {
                    text: "+"

                    onClicked:
                    {
                        configFieldsPopup.storeFields()
                        loadStoredFieldsTimer.start() //Hot fix for ReferenceError: xyz is not defined
                        configFieldsRepeater.model = configFieldsRepeater.model + 1 //WE got issue here as our current object is child
                    }
                }
            }
        }

        Timer
        {
            id: loadStoredFieldsTimer
            interval: 50
            running: false
            repeat: false
            onTriggered: {
                configFieldsPopup.loadFields()
            }
        }

        Button
        {
            text: "Add new"

            x: parent.width/2 - width/2 - 5
            y: parent.height - height - 5

            onClicked:
            {
                configFieldsPopup.storeFields()
                loadStoredFieldsTimer.start()
                configFieldsRepeater.model += 1 //Как в HD версии
            }
        }


        Button
        {
            text: "Close"

            x: parent.width - width - 10
            y: parent.height - height - 5

            onClicked: configFieldsPopup.close()
        }

        Button
        {
            text: "Save"

            x: 10
            y: parent.height - height - 5

            onClicked:
            {
                configFieldsPopup.lastVisualReport.clearReportFields()

                for (var i = 0; i < configFieldsRepeater.model; ++i)
                {

                    var name = configFieldsRepeater.itemAt(i).getName()
                    var color = configFieldsRepeater.itemAt(i).getColor()
                    var yCoef = parseFloat(configFieldsRepeater.itemAt(i).getYCoef())

                    configFieldsPopup.lastVisualReport.addReportField(name, color, yCoef)
                }

                configFieldsPopup.lastVisualReport.setReportFieldsType()
                configFieldsPopup.close()
            }
        }
    }



    JsonReport
    {
        id: jsonReport

        Component.onCompleted:
        {

        }

    }


    function keyboardEventSend(key, mode) {
        //Заглушка, но можно реализовать логику здесь
    }

}
