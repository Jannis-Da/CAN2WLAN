const socket = new WebSocket('ws://' + window.location.hostname + '/ws');

var canFrames = [];

function populateTable(){
    var tableBody = document.getElementById('canTable').getElementsByTagName('tbody')[0];

    tableBody.innerHTML = '';

    for (var i = 0; i < canFrames.length; i++) {
        var row = tableBody.insertRow(i);

        var idCell = row.insertCell(0);
        var frameLengthCell = row.insertCell(1);
        var dataCell = row.insertCell(2);
        var extCell = row.insertCell(3);
        var rtrCell = row.insertCell(4);

        idCell.innerHTML = canFrames[i].identifier;
        frameLengthCell.innerHTML = canFrames[i].frameLength;
        dataCell.innerHTML = canFrames[i].data;
        extCell.innerHTML = canFrames[i].extFlag;
        rtrCell.innerHTML = canFrames[i].rtrFlag;
    }
}


socket.onopen = function(event) {
    console.log('WebSocket connection opened:', event);
};

socket.onmessage = function(event) {
    console.log('WebSocket message received:', event.data);

    try {
        const canData = JSON.parse(event.data);
        
        canFrames.unshift(canData);

        populateTable();

    } catch (error) {
        console.error('Error parsing JSON:', error);
    }
};

socket.onclose = function(event) {
    console.log('WebSocket connection closed:', event);
};

socket.onerror = function(event) {
    console.error('WebSocket error:', event);
    console.error('WebSocket readyState:', socket.readyState);
};


function exportToXML() {
    // Create an XML string with the received CAN messages
    var xmlString = '<?xml version="1.0" encoding="UTF-8"?>\n<CanMessages>\n';

    for (var i = 0; i < canFrames.length; i++) {
        xmlString += '    <CanMessage>\n';
        xmlString += `        <ID>${canFrames[i].identifier}</ID>\n`;
        xmlString += `        <FrameLength>${canFrames[i].frameLength}</FrameLength>\n`;
        xmlString += `        <Data>${canFrames[i].data}</Data>\n`;
        xmlString += `        <ExtendedFlag>${canFrames[i].extFlag}</ExtendedFlag>\n`;
        xmlString += `        <RTRFlag>${canFrames[i].rtrFlag}</RTRFlag>\n`;
        xmlString += '    </CanMessage>\n';
    }

    xmlString += '</CanMessages>';

    // Create a Blob with the XML data
    var blob = new Blob([xmlString], { type: 'text/xml' });

    // Create a link to trigger the download
    var a = document.createElement('a');
    a.href = window.URL.createObjectURL(blob);
    a.download = 'CAN_Dump.xml';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
}

function configureCAN() {
    // Retrieve values from the input fields and configure CAN communication
    var canSpeed = document.getElementById('canSpeed').value;
    var filter = document.getElementById('filter').value;

    // Perform necessary actions with the configured values
    console.log('Configuring CAN with Speed:', canSpeed, 'and Filter:', filter);
}