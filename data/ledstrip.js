/**
 * @Authors:        Laurens Leusink
 * @Date created:   6-1-2024
 * @Date updated:   7-1-2024 (By: Laurens Leusink)
 * @description:    Processing the GPIO pins and segments of the LED strip.
 */

// global variables.
let segments = [];
let gpios = [];  // 0, 1, 2, 3, 4, 5, 12, 13
let editingSegment = null;
let isEditing = [];

/**
 * Format the segments to be sent to the server.
 * @param {object} segments 
 * @returns {object}
 */
function formatJsonResponse(segments) {
    return {
        "segments": segments.map(segment => ({
            "start": segment.start,
            "len": segment.length,
            "pin": segment.GPIO,
            "rev": segment.reversed,
        }))
    };
}

/**
 * Converts the segments and stores them in the server.
 */
function saveChanges() {
    const formatted_data = JSON.stringify(formatJsonResponse(segments));
    setSegments(formatted_data);
    rerenderGPIOs();
}

/**
 * Gets the segments from the server and stores them
 */
function loadChanges() {
    getSegments().then(data => {
        if (!data || !data.segments) {
            alert("Invalid data received from server.");
            return;
        }

        segments = data.segments;
        gpios = data.gpios || [];

        // Retreive the gpios from the data.
        data.segments.forEach(segment => {
            if (!gpios.includes(segment.pin[0])) {
                gpios.push(segment.pin[0]);
            }
        });

        // Retreive the segments from the data.
        segments = data.segments.map((segment, index) => {
            return {
                id: index,
                start: segment.start,
                length: segment.len,
                GPIO: segment.pin[0],
                reversed: segment.rev
            }
        });
        rerenderGPIOs();
    }).catch(error => {
        alert("Error loading segments");
    });
}

/**
 * Change the editing state of a GPIO.
 * @param {int} GPIOid 
 */
function setGPIOEditing(GPIOid) {
    isEditing.push(GPIOid)
    rerenderGPIOs()
}

/**
 * Saves the changed segments of the selected GPIO
 * @param {int} GPIOid 
 * @returns 
 */
function saveGPIO(GPIOid) {
    isEditing = isEditing.filter(_id => _id !== GPIOid)

    const GPIOs = sortOnGPIO(segments);
    const toSaveSegments = GPIOs[GPIOid];

    if (!toSaveSegments) return rerenderGPIOs(); // Check if the GPIO has any segments

    // Update the segments in the html
    segments = segments.map(segment => {
        const toUpdate = toSaveSegments.find(s => s.id === segment.id);
        if (toUpdate) {
            const newStart = parseInt(document.getElementById(`ledstrip_seg${toUpdate.id}_start`).value);
            const newLength = parseInt(document.getElementById(`ledstrip_seg${toUpdate.id}_len`).value);
            const newReversed = document.getElementById(`ledstrip_seg${toUpdate.id}_rev`).checked;

            // Check for overlapping segments on the same pin and other pins
            for (let otherSegment of segments) {
                if (otherSegment.id !== segment.id && (
                    (newStart >= otherSegment.start && newStart < otherSegment.start + otherSegment.length) ||
                    (newStart + newLength > otherSegment.start && newStart + newLength <= otherSegment.start + otherSegment.length) ||
                    (newStart <= otherSegment.start && newStart + newLength >= otherSegment.start + otherSegment.length)
                )) {
                    alert(`Segment overlaps with an existing segment on pin ${otherSegment.GPIO}.`);
                    return segment;
                }
            }

            return {
                ...segment,
                start: newStart,
                length: newLength,
                reversed: newReversed,
            };
        }
        return segment;
    });

    saveChanges();
}

/**
 * Adds GPIO pins to the HTML page
 */
function rerenderGPIOs() {
    const container = document.getElementById("ledstrip_content");
    container.innerHTML = '';
    const GPIOs = sortOnGPIO(segments);

    gpios.forEach(gpio => {
        let GPIOSegments = GPIOs[gpio];
        GPIOSegments = GPIOSegments ? GPIOSegments.sort((a, b) => a.start - b.start) : [];
        
        const doEdit = isEditing.some(_id => _id == gpio);
        const strip = createStrip(gpio, GPIOSegments, doEdit);
    
        container.appendChild(strip);
        if (doEdit) document.getElementById(`ledstrip_gpio${gpio}_start`).focus();
      });
}

/**
 * Stores a new segment in the server
 */
function addNewSegment(GPIO, newStart, newLength, newReversed) {
    for (let segment of segments) {
        if (
            (newStart >= segment.start && newStart < segment.start + segment.length) ||
            (newStart + newLength > segment.start && newStart + newLength <= segment.start + segment.length) ||
            (newStart <= segment.start && newStart + newLength >= segment.start + segment.length)
        ) {
            alert(`Segment overlaps with an existing segment on pin ${segment.GPIO}.`);
            return;
        }
    }

    let nextId = segments.reduce((maxId, segment) => Math.max(maxId, segment.id), 0) + 1;
    const seg = { "id": nextId, "start": newStart, "length": newLength, "GPIO": GPIO, "reversed": newReversed }
    segments.push(seg)

    saveChanges();
}

/**
 * Deletes a segment from the server
 * @param {int} id
 */
function deleteSegment(id) {
    segments = segments.filter(segment => segment.id !== id);

    saveChanges();
}

/**
 * Deletes a GPIO pin from HTML
 * @param {int} gpio 
 */
function handleDeleteGPIO(gpio) {
    gpios = gpios.filter(g => g !== gpio);

    for (let i = 0; i < segments.length; i++) {
        const segment = segments[i];
        if (segment.GPIO == gpio) deleteSegment(segment.id);
    }

    saveChanges();
}

/**
 * Checks if new added GPIO is valid
 */
function handleAddGPIO() {
    value = parseInt(document.getElementById("newGPIO").value);

    if (isNaN(value)) {
        alert('Unable to parse gpio number.');
        return;
    }
    if (gpios.includes(value)) {
        alert('GPIO already exists.');
        return;
    }

    gpios.push(value);

    saveChanges();
}

/**
 * Sorts the segments for each GPIO
 */
function sortOnGPIO() {
    const GPIOs = {};

    for (let i = 0; i < segments.length; i++) {
        const segment = segments[i];
        const GPIO = segment.GPIO;

        if (GPIOs[GPIO]) GPIOs[GPIO].push(segment);
        else GPIOs[GPIO] = [segment];
    }

    return GPIOs;
}

/**
 * Adds a GPIO to the list of GPIOs
 */
function addGPIO(){
    const inputField = document.getElementById('newGPIO');
    const GPIO = parseInt(inputField.value);
    if (isNaN(GPIO)) return;

    gpios.push(GPIO);

    inputField.value = '';
}

/**
 * Override the default alert function
 * @param {String} message 
 */
window.alert = function(message) {
    const alertContainer = document.getElementById('alertContainer');
    if (!alertContainer) return;

    const alertDiv = document.createElement('div');
    alertDiv.className = 'alert alert-danger alert-dismissible fade show';
    alertDiv.role = 'alert';
    alertDiv.innerHTML = `
    ${message}
    <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close">X</button>
    `;

    alertContainer.appendChild(alertDiv);

    alertDiv.querySelector('.btn-close').addEventListener('click', () => {
        alertDiv.classList.remove('show');
        alertDiv.addEventListener('transitionend', () => alertDiv.remove());
    });

    setTimeout(() => {
        alertDiv.classList.remove('show');
        alertDiv.addEventListener('transitionend', () => alertDiv.remove());
    }, 10000); // Alert display time in ms
};

loadChanges()
rerenderGPIOs()