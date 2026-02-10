/**
 * @Authors:        Laurens Leusink
 * @Date created:   6-1-2024
 * @Date updated:   7-1-2024 (By: Laurens Leusink)
 * @description:    Handling the API calls
 */

/**
 * Reset network settings
 */
function resetNetworkSettings(){
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "resetNetworkSettings", true);
    xhr.onload = function(){
        if(xhr.status === 200){
            location.reload();
        }
    }
    xhr.send();
}

/**
 * Updates the network settings
 * @param {String} ip
 * @param {String} gateway
 * @param {String} subnet
 * @param {String} hostname
 */

function updateNetworkSettings(ip, gateway, subnet, hostname){
    var xhr = new XMLHttpRequest();
    xhr.open("POST", "updateNetwork?ip=" + ip + "&gateway=" + gateway + "&subnet=" + subnet + "&hostname=" + hostname, true);
    xhr.onload = function(){
        if(xhr.status === 200){
            location.reload();
        }
    }
    xhr.send();
}

function getNetworkSettings(){
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "GetNetworkSettings", true);
    xhr.onload = function() {
        if (xhr.status === 200) {
            try {
                var jsonResponse = JSON.parse(xhr.responseText);
                resolve(jsonResponse);
            } catch (e) {
                console.error("Error parsing JSON: ", e);
                reject(e);
            }
        } else {
            console.error("Failed to get segments, status: ", xhr.status);
            reject(new Error(`Failed to get segments, status: ${xhr.status}`));
        }
        xhr.send();
    };
}

/**
 * Sets the segments to the server
 * @param {*} segments
 */
function setSegments(segments) {
    const xhr = new XMLHttpRequest();
    xhr.open("POST", "/setSegments", true);
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.onload = function() {
        if (xhr.status === 200) {
        } else {
            alert("Failed to update segments");
        }
    };
    xhr.send(segments);
}

/**
 * Gets the segments from the server
 */
function getSegments() {
    return new Promise((resolve, reject) => {
        console.debug("Getting segments");
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/getSegments", true);

        xhr.onload = function() {
            if (xhr.status === 200) {
                try {
                    var jsonResponse = JSON.parse(xhr.responseText);
                    resolve(jsonResponse);
                } catch (e) {
                    console.error("Error parsing JSON: ", e);
                    reject(e);
                }
            } else {
                console.error("Failed to get segments, status: ", xhr.status);
                reject(new Error(`Failed to get segments, status: ${xhr.status}`));
            }
        };
        xhr.send();
    });
}