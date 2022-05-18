import QtQuick 2.12
import diaryth 1.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1


Item {

    id: fullHDReport


    Component.onCompleted:
    {

    }


    JsonReport
    {
        id: jsonReport

        Component.onCompleted:
        {
            reloadVisualReports()
        }

    }

    function reloadVisualReports()
    {
        flick.contentWidth = jsonReport.getFullWidth()
        for (var i = 0; i < reportsRepeater.model; ++i)
        {
            reportsRepeater.itemAt(i).setParent(jsonReport)
            reportsRepeater.itemAt(i).width = jsonReport.getFullWidth()
            reportsRepeater.itemAt(i).setType(fullHDReport.reportsTypes[i])
        }
    }


    property var reportsHeight: [ 200, 35, 200, 500 ]
    property var reportsTypes: [VisualTypes.PraatInfo, VisualTypes.PlainWords, VisualTypes.ReportFields, VisualTypes.Pitch]


    function calculateY(idx)
    {
        var hSum = 0
        for (var i = 1; i <= idx; ++i)
            hSum += reportsHeight[i - 1]

        //console.log("ON idx ", idx, " we got ", hSum)

        return hSum + 5 * (idx + 1)
    }

    RowLayout
    {
        x: 25
        y: 10

        Button {
            text: "text"
        }
    }


    ScrollView
    {
        y: 50

        id: scroll
        width: parent.width
        height: fullHDReport.reportsHeight[0] + reportsHeight[1] + reportsHeight[2] + reportsHeight[3]

        function updatePositions()
        {
            //TODO rewrite with cycle
            scroll.height = fullHDReport.reportsHeight[0] + reportsHeight[1] + reportsHeight[2] + reportsHeight[3]
            flick.height = scroll.height

            for (var i = 0; i < reportsRepeater.model; ++i)
                reportsRepeater.itemAt(i).y = fullHDReport.calculateY(i)

        }

        ScrollBar {
            orientation: Qt.Horizontal
            height: 50
        }

        ScrollBar.horizontal.interactive: true

        Flickable
        {
            id: flick
            y: 5
            x: 0
            width: parent.width
            height: scroll.height
            contentWidth: 3000
            contentHeight:  parent.height
            property int pressedX : 0


            Repeater
            {
                id: reportsRepeater

                model: 4

                VisualReport
                {
                    id: visualReport
                    height:  fullHDReport.reportsHeight[index]
                    width: 3000
                    y:  fullHDReport.calculateY(index)

                    MouseArea
                    {
                        anchors.fill: parent
                        acceptedButtons: Qt.LeftButton | Qt.RightButton

                        onClicked:
                        {

                        }
                        onDoubleClicked:
                        {
                            if (mouse.button == Qt.RightButton)
                            {
                                console.log("Right mouse dbl click")
                                var reportType = visualReport.getType()

                                if (reportType === VisualTypes.PraatInfo || reportType === VisualTypes.PraatInfoFullDiff ||
                                    reportType === VisualTypes.PraatInfoChunkDiff || reportType === VisualTypes.ChunksOnly)
                                {
                                    praatFieldsConfigPopup.loadFromVisual(visualReport)
                                    praatFieldsConfigPopup.open()
                                }

                                if (reportType === VisualTypes.ReportFields)
                                {
                                    reportFieldsConfigPopup.loadFromVisual(visualReport)
                                    reportFieldsConfigPopup.open()
                                }

                            }
                            else
                            {
                                var idx = jsonReport.eventIdxOnClick(mouseX, mouseY)
                                jsonReport.selectEvent(idx)
                            }
                        }

                        onPressAndHold:
                        {
                            if (mouse.button == Qt.RightButton)
                            {
                                configVisualReportPopup.connectWithReport(visualReport, index)
                                configVisualReportPopup.open()
                            }
                        }

                    }
                }


            }

        } //Flickable

    } //ScrollView



//================Configure single report view=============================================

    Popup
    {
        id: configVisualReportPopup

        x: fullHDReport.width/2 - width/2 - 25
        y: fullHDReport.height - height - 50

        width: 500
        height: 160

        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        property var lastVisualReport: "" //undefined?
        property int reportIndex: 0

        function connectWithReport(visualReport, idx)
        {
            configVisualReportPopup.lastVisualReport = visualReport
            configVisualReportPopup.reportIndex = idx

            var reportType = visualReport.getType()
            visualReportType.currentIndex = reportType - 1
            reportHeightSlider.value = visualReport.height

            for (var i = 0; i < reportsRepeater.model; ++i)
                reportsRepeater.itemAt(i).setShowBorder(true)
        }

        onClosed:
        {
            for (var i = 0; i < reportsRepeater.model; ++i)
                reportsRepeater.itemAt(i).setShowBorder(false)
        }


        ComboBox
        {
            y: 20
            x: 25
            width: parent.width - 50

            id: visualReportType
            model: ["Pitch", "Intensity", "Praat", "PraatChunkDiff", "PraatFullDiff",
                    "Chunks", "Words", "ReportFields"]

            onCurrentTextChanged: {
                if (configVisualReportPopup.lastVisualReport !== "")
                    configVisualReportPopup.lastVisualReport.setType(currentIndex + 1)
            }
        }

        Slider
        {
            id: reportHeightSlider

            y: 30 + visualReportType.height

            x: 25
            width: parent.width - 50

            from: 35
            to: 600
            stepSize: 5

            value: 190

            onMoved:
            {
                fullHDReport.reportsHeight[configVisualReportPopup.reportIndex] = value
                configVisualReportPopup.lastVisualReport.height = value
                scroll.updatePositions()
            }
        }

    }



//================Configure single report view=============================================



//=====Visual reports fields popups========================================================
//=========================================================================================


    Popup //TODO into sepparated component
    {
        id: praatFieldsConfigPopup
        x: 50
        y: 50
        width: fullHDReport.width - x * 2
        height: fullHDReport.height - x * 2
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
            praatFieldsConfigPopup.lastVisualReport = visualReport

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

            praatFieldsConfigPopup.storedFields = fieldsList
        }

        function loadFields()
        {
            for (var i = 0; i < praatFieldsConfigPopup.storedFields.length; ++i)
            {
                var fieldLine = praatFieldsConfigPopup.storedFields[i]
                configRepeater.itemAt(i).loadValues(fieldLine[0], fieldLine[1], fieldLine[2])
            }
        }



        Repeater
        {
            model: 3
            id: configRepeater //TODO cover under flickable

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

                    for (i = 0; i < praatFieldsConfigPopup.colorsNames.length; ++i)
                    {
                        if (praatFieldsConfigPopup.colorsNames[i] === color)
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
                    model: praatFieldsConfigPopup.colorsNames
                }

                TextField {
                    id: fieldCoef
                    validator: realValidator
                    text: "1,0"
                }

                RoundButton {
                    text: "-"

                    onClicked: {
                        praatFieldsConfigPopup.storeFields(index)
                        loadStoredTimer.start() //Hot fix for ReferenceError: xyz is not defined
                        configRepeater.model = configRepeater.model - 1 //WE got issue here as our current object is child
                    }
                }

                RoundButton {
                    text: "+"

                    onClicked:
                    {
                        praatFieldsConfigPopup.storeFields()
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
                praatFieldsConfigPopup.loadFields()
            }
        }

        Button
        {
            text: "Add new"

            x: parent.width/2 - width/2 - 5
            y: parent.height - height - 5

            onClicked:
            {
                praatFieldsConfigPopup.storeFields()
                loadStoredTimer.start() //Hot fix for ReferenceError: xyz is not defined
                configRepeater.model += 1 //TODO решени этого хотфикса можно сделать за счёт запуска таймера с инкрементом и декриментом
            }
        }


        Button
        {
            text: "Close"

            x: parent.width - width - 10
            y: parent.height - height - 5

            onClicked: praatFieldsConfigPopup.close()
        }

        Button
        {
            text: "Save"

            x: 10
            y: parent.height - height - 5

            onClicked:
            {
                praatFieldsConfigPopup.lastVisualReport.clearPraatFields() //find a way to use parametric values

                for (var i = 0; i < configRepeater.model; ++i)
                {

                    var name = configRepeater.itemAt(i).getName()
                    var color = configRepeater.itemAt(i).getColor()
                    var yCoef = parseFloat(configRepeater.itemAt(i).getYCoef())

                    praatFieldsConfigPopup.lastVisualReport.addPraatField(name, color, yCoef)
                }

                praatFieldsConfigPopup.lastVisualReport.setPraatType() //way to repaint

                praatFieldsConfigPopup.close()
            }
        }
    }




    Popup
    {
        id: reportFieldsConfigPopup
        x: 50
        y: 50
        width: fullHDReport.width - x * 2
        height: fullHDReport.height - x * 2
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
            reportFieldsConfigPopup.lastVisualReport = visualReport
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

            reportFieldsConfigPopup.storedFields = fieldsList
        }

        function loadFields()
        {
            for (var i = 0; i < reportFieldsConfigPopup.storedFields.length; ++i)
            {
                var fieldLine = reportFieldsConfigPopup.storedFields[i]
                configFieldsRepeater.itemAt(i).loadValues(fieldLine[0], fieldLine[1], fieldLine[2])
            }
        }



        Repeater
        {
            model: 3
            id: configFieldsRepeater //TODO cover under flickable

            RowLayout
            {
                y: index * 60

                function loadValues(name, color, coef)
                {
                    praatFieldName2.text = name
                    fieldCoef2.text = coef

                    for (var i = 0; i < reportFieldsConfigPopup.colorsNames.length; ++i)
                        if (reportFieldsConfigPopup.colorsNames[i] === color)
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
                    model: reportFieldsConfigPopup.colorsNames
                }

                TextField {
                    id: fieldCoef2
                    validator: realValidator
                    text: "1,0"
                }

                RoundButton {
                    text: "-"

                    onClicked: {
                        reportFieldsConfigPopup.storeFields(index)
                        loadStoredFieldsTimer.start() //Hot fix for ReferenceError: xyz is not defined
                        configFieldsRepeater.model = configFieldsRepeater.model - 1 //WE got issue here as our current object is child
                    }
                }

                RoundButton {
                    text: "+"

                    onClicked:
                    {
                        reportFieldsConfigPopup.storeFields()
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
                reportFieldsConfigPopup.loadFields()
            }
        }

        Button
        {
            text: "Add new"

            x: parent.width/2 - width/2 - 5
            y: parent.height - height - 5

            onClicked:
            {
                reportFieldsConfigPopup.storeFields()
                loadStoredFieldsTimer.start() //Hot fix for ReferenceError: xyz is not defined
                configFieldsRepeater.model += 1 //TODO решени этого хотфикса можно сделать за счёт запуска таймера с инкрементом и декриментом
            }
        }


        Button
        {
            text: "Close"

            x: parent.width - width - 10
            y: parent.height - height - 5

            onClicked: reportFieldsConfigPopup.close()
        }

        Button
        {
            text: "Save"

            x: 10
            y: parent.height - height - 5

            onClicked:
            {
                reportFieldsConfigPopup.lastVisualReport.clearReportFields()

                for (var i = 0; i < configFieldsRepeater.model; ++i)
                {

                    var name = configFieldsRepeater.itemAt(i).getName()
                    var color = configFieldsRepeater.itemAt(i).getColor()
                    var yCoef = parseFloat(configFieldsRepeater.itemAt(i).getYCoef())

                    reportFieldsConfigPopup.lastVisualReport.addReportField(name, color, yCoef)
                }

                reportFieldsConfigPopup.lastVisualReport.setReportFieldsType()
                reportFieldsConfigPopup.close()
            }
        }
    }




//=====Visual reports fields popups========================================================
//=========================================================================================


}
