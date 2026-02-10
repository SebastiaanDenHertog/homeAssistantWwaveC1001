/**
 * @Authors:        Laurens Leusink
 * @Date created:   6-1-2024
 * @Date updated:   7-1-2024 (By: Laurens Leusink)
 * @description:    Creates the necessary HTML for the GPIO pins and their segments
 */

/**
 * Creates the HTML for the GPIO pins and their segments
 */
function createStrip(GPIO, segmentsData, isEditing) {
    const strip = document.createElement('div');
    strip.classList.add('strip');
  
    const header = document.createElement("div");
    header.className = "header"
  
    const heading = document.createElement('h3');
    heading.textContent = `Pin ${GPIO}`;
    header.appendChild(heading);
    
    if (isEditing) {  
      const deleteGPIOButton = document.createElement('button');
      deleteGPIOButton.classList.add('small', 'btn', 'btn-danger');
      deleteGPIOButton.innerText = 'DEL';
      deleteGPIOButton.onclick = function () { handleDeleteGPIO(GPIO) };
      header.appendChild(deleteGPIOButton);
  
      const saveButton = document.createElement('button');
      saveButton.classList.add('small', 'btn', 'btn-success');
      saveButton.innerText = 'SAVE';
      saveButton.onclick = function () { saveGPIO(GPIO) };
      header.appendChild(saveButton);
    } else {
      const editButton = document.createElement('button');
      editButton.classList.add('small', 'btn', 'btn-primary');
      editButton.innerText = 'EDIT';
      editButton.onclick = function () { setGPIOEditing(GPIO) };
      header.appendChild(editButton);
    }
  
    strip.appendChild(header);
  
    const segments = document.createElement('div');
    segments.classList.add('segments');
  
    if (isEditing) {
      segments.classList.add('editing');
    }

function addSegment(id, start, length, reversed) {
  if (isEditing) {
    const inputStart = document.createElement('input');
    inputStart.id = `ledstrip_seg${id}_start`;
    inputStart.value = start;
    inputStart.className = "fancy"
    inputStart.maxLength = 4;
    segments.appendChild(inputStart);

    const inputLength = document.createElement('input');
    inputLength.id = `ledstrip_seg${id}_len`;
    inputLength.value = length;
    inputLength.className = "fancy"
    inputLength.maxLength = 4;
    segments.appendChild(inputLength);

    const directionBox = document.createElement('input');
    directionBox.type = "checkbox"
    directionBox.id = `ledstrip_seg${id}_rev`;
    directionBox.className = "fancy";
    directionBox.checked = reversed;
    segments.appendChild(directionBox);

    const delButton = document.createElement('button');
    delButton.classList.add('small', 'btn', 'btn-danger');
    delButton.innerText = 'DEL';
    delButton.onclick = function () { deleteSegment(id) };
    segments.appendChild(delButton);
  } else {
    const spanId = document.createElement('span');
    spanId.className = "sub"
    segments.appendChild(spanId);

    const spanStart = document.createElement('span');
    spanStart.textContent = start;
    segments.appendChild(spanStart);

    const dash = document.createElement('span');
    dash.textContent = '-';
    segments.appendChild(dash);

    const spanEnd = document.createElement('span');
    spanEnd.textContent = start + length;
    segments.appendChild(spanEnd);

    const spanTotal = document.createElement('span');
    spanTotal.className = "sub"
    spanTotal.textContent = `(${length})`;
    segments.appendChild(spanTotal);

    const direction = document.createElement('span');
    direction.className = reversed ? "green" : "red";
    direction.innerText = reversed ? "✔" : "✘";
    segments.appendChild(direction);
  }
}

if (isEditing) {
  const spanStart = document.createElement('span');
  spanStart.textContent = "Start";

  const spanLength = document.createElement('span');
  spanLength.textContent = "Length";

  const spanRev = document.createElement('span');
  spanRev.textContent = "Rev";

  const spanDel = document.createElement('span');
  spanDel.textContent = "Delete";

  segments.appendChild(spanStart);
  segments.appendChild(spanLength);
  segments.appendChild(spanRev);
  segments.appendChild(spanDel);
}
else {
  const spanStart = document.createElement('span');
  spanStart.textContent = "Start";

  const dash = document.createElement('span');
  dash.textContent = '-';

  const spanEnd = document.createElement('span');
  spanEnd.textContent = "End";

  const lengthText = document.createElement('span');
  lengthText.textContent = "Length";

  const revText = document.createElement('span');
  revText.textContent = "Reversed";

  segments.appendChild(spanStart);
  segments.appendChild(dash);
  segments.appendChild(spanEnd);
  segments.appendChild(lengthText);
  segments.appendChild(revText);
}


segmentsData.forEach(segmentData => {
  addSegment(
    segmentData.id,
    segmentData.start,
    segmentData.length,
    segmentData.reversed
  )
});

strip.appendChild(segments);

if (isEditing) {
  const addSegmentDiv = document.createElement('div');
  addSegmentDiv.classList.add('add-segment');

  const inputStart = document.createElement('input');
  inputStart.id = `ledstrip_gpio${GPIO}_start`;
  inputStart.className = "fancy"
  inputStart.maxLength = 4;
  addSegmentDiv.appendChild(inputStart);

  const inputLength = document.createElement('input');
  inputLength.id = `ledstrip_gpio${GPIO}_len`;
  inputLength.className = "fancy"
  inputLength.maxLength = 4;
  addSegmentDiv.appendChild(inputLength);

  const directionBox = document.createElement('input');
  directionBox.type = "checkbox"
  directionBox.id = `ledstrip_gpio${GPIO}_rev`;
  directionBox.className = "fancy";
  addSegmentDiv.appendChild(directionBox);

  const editButton = document.createElement('button');
  editButton.classList.add('small', 'btn', 'btn-primary');
  editButton.innerText = 'ADD';
  editButton.onclick = function () {
    const newStart = parseInt(document.getElementById(`ledstrip_gpio${GPIO}_start`).value);
    const newLength = parseInt(document.getElementById(`ledstrip_gpio${GPIO}_len`).value);
    const newReversed = document.getElementById(`ledstrip_gpio${GPIO}_rev`).checked;

    if (isNaN(newStart) || isNaN(newLength)) return
    addNewSegment(GPIO, newStart, newLength, newReversed)
  };
  addSegmentDiv.appendChild(editButton);

  const hr = document.createElement("hr")

  strip.appendChild(hr)
  strip.appendChild(addSegmentDiv)
}

return strip;
}

function createGPIOItem(GPIO) {
  const div = document.createElement('div');
  div.className = "gpio-item";

  const span = document.createElement('span');
  span.innerText = GPIO;

  const button = document.createElement('button');
  button.className = 'small btn btn-danger';
  button.innerText = 'DEL';
  button.onclick = function () { handleDeleteGPIO(GPIO) };

  div.appendChild(span);
  div.appendChild(button);

  return div;
}

var ip = document.getElementById('ip').value;
if (ip === "0.0.0.0") {
  document.getElementById('dhcp').checked = true;
  var networkForm = document.getElementById('networkForm');
  networkForm.style.display = 'none';
}


/**
 * Toggles DHCP: if checked, disable IP/Gateway/Subnet,
 * but ALWAYS keep Hostname editable.
 * Also displays a small status message.
 */
function toggleDHCP() {
  const isChecked = document.getElementById('dhcp').checked;

  // Enable/disable IP, Gateway, Subnet
  document.getElementById('rowIP').style.display      = isChecked ? 'none' : 'table-row';
  document.getElementById('rowGateway').style.display = isChecked ? 'none' : 'table-row';
  document.getElementById('rowSubnet').style.display  = isChecked ? 'none' : 'table-row';


  // Hostname always active
  document.getElementById('hostname').disabled = false;

  // Show some user feedback
  const dhcpStatus = document.getElementById('dhcpStatus');
  dhcpStatus.style.display = 'block';
}