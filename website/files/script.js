/*
 * Account page
 */

function validUser(username) {
	if (username.length < 4 || username.length > 30)
		return false;

	if (!username.match(/^[A-Za-z0-9\-_\.]+$/))
		return false;

	return true;
}

function password(pass) {
    var salt = "freetron";
    var result = Sha256.hash(salt + pass);

    return result;
}

function forgotMouseOver() {
    var box  = $("forgotmsg");
    box.style.display = "block";
}

function forgotMouseOut() {
    var box  = $("forgotmsg");
    box.style.display = "none";
}

function deleteAccount() {
    var result = confirm("Are you sure you want to delete your account? All of "+
            "your data will be permanently removed.");

    if (result) {
        var confirmationCode = parseInt($("confirm").value, 10);
        window.rpc.account_delete.on_result = function(r) { if (r) goHome(); };
        window.rpc.account_delete(confirmationCode);
    }

    return false;
}

function accountSubmit() {
    var user = $("user");
    var pass = $("pass");
    var badlogin = $("badlogin");

    if (validUser(user.value)) {
        badlogin.style.display = "none";
        var hash = password(pass.value);

        window.rpc.account_login.on_error = function(e) {
            var badlogin = $("badlogin");
            badlogin.style.display = "inline";
        };
        window.rpc.account_login.on_result = function(r) {
            var badlogin = $("badlogin");

            if (r) {
                badlogin.style.display = "none";
                goHome();
            } else {
                badlogin.style.display = "inline";
            }
        };
        window.rpc.account_login(user.value, hash);
    } else {
        badlogin.style.display = "inline";
    }

    return false;
}

function newAccountSubmit() {
    var badusername = $("badusername");
    var new_user = $("new_user");
    var new_pass = $("new_pass");
    var username = new_user.value;

    if (!validUser(username)) {
        badusername.style.display = "inline";
        new_user.className = "new_user";
    } else {
        badusername.style.display = "none";
        new_user.className = "field";

        var hash = password(new_pass.value);

        window.rpc.account_create.on_error = function(e) {
            var badusername = $("badusername");
            var new_user = $("new_user");
            badusername.style.display = "inline";
            new_user.className = "new_user";
        };
        window.rpc.account_create.on_result = function(r) {
            var badusername = $("badusername");
            var new_user = $("new_user");

            if (r) {
                badusername.style.display = "none";
                new_user.className = "field";

                goHome();
            } else {
                badusername.style.display = "inline";
                new_user.className = "new_user";
            }
        };
        window.rpc.account_create(new_user.value, hash);
    }

    return false;
}

function updateAccountSubmit() {
    var badusername = $("badusernameupdate");
    var update_user = $("update_user");
    var update_pass = $("update_pass");
    var username = update_user.value;

    if (!validUser(username)) {
        badusername.style.display = "inline";
        update_user.className = "new_user";
    } else {
        badusername.style.display = "none";
        update_user.className = "field";

        var hash = password(update_pass.value);

        window.rpc.account_update.on_error = function(e) {
            var badusername = $("badusernameupdate");
            var update_user = $("update_user");
            badusername.style.display = "inline";
            update_user.className = "new_user";
        };
        window.rpc.account_update.on_result = function(r) {
            var badusername = $("badusernameupdate");
            var update_user = $("update_user");
            var update_pass = $("update_pass");

            if (r) {
                badusername.style.display = "none";
                update_user.className = "field";

                update_pass.value = "";
            } else {
                badusername.style.display = "inline";
                update_user.className = "new_user";
            }
        };
        window.rpc.account_update(update_user.value, hash);
    }

    return false;
}

/*
 * Forms page
 */

function isPdf(type, name) {
    return type === "application/pdf" || name.substr(-3,3).toLowerCase() === "pdf";
}

function validKey(key) {
	if (key.length < 1 || key.length > 10)
		return false;

	if (!key.match(/^[0-9]+$/))
		return false;

	return true;
}

// Mostly from http://matlus.com/html5-file-upload-with-progress/
function fileSelected() {
    var error = $('fileError');
    var file = $('uploadFile').files[0];

    if (file) {
        if (isPdf(file.type, file.name)) {
            error.innerHTML = "";
        } else {
            fileError("Must be PDF");
        }
    }
}

// Initiate the upload
function uploadFile() {
    var file = $('uploadFile').files[0];
    var error = $('fileError');
    var progress = $('progress');
    var button = $('uploadFileButton');
    var key = $('key').value;

    if (!file) {
        fileError("Invalid file");
    } else if (!validKey(key)) {
        fileError("Key must be a number");
    } else if (!isPdf(file.type, file.name)) {
        fileError("Must be PDF");
    } else {
        var fd;

        // If this doesn't work at some point, look at
        // https://developer.mozilla.org/en-US/docs/Web/Guide/HTML/Forms/Sending_forms_through_JavaScript
        var form = $('upload');

        if (typeof form.getFormData  === "function") {
            fd = form.getFormData();
        } else if (typeof FormData === "function") {
            fd = new FormData(form);
        } else {
            // TODO: implement support
            alert("browser not supported yet");
            return false;
        }

        http("/upload/" + key,
        uploadComplete, function(data) {
            fileError("Error uploading file");
        }, uploadProgress, function(evt) {
            fileError("Canceled");
        }, fd);

        error.innerHTML = "";
        button.disabled = true;
        window.needToConfirm = true;
    }

    return false;
}

// Create a request to see the processing status
function monitorProcessing(id) {
    window.rpc.form_process.on_error = function(e) {
        fileError("Error processing file");
    };
    window.rpc.form_process.on_result = function(r) {
        var progress = $('progress');
        progress.innerHTML = "Processing: " + r["percent"] + '%';

        if (r["percent"] == 100) {
            $("upload").reset();
            progress.innerHTML = "Done";

            // Get result
            formGetOne(id);

            setTimeout(function() {
                if (progress.innerHTML === "Done")
                    clearProgress();
            }, 3000);
        } else {
            // Wait for next status update
            monitorProcessing(id);
        }
    };
    window.rpc.form_process(id);
}

// Print status messages
function uploadProgress(evt) {
    var progress = $('progress');

    if (evt.lengthComputable) {
        var percentComplete = Math.round(evt.loaded * 100 / evt.total);
        progress.innerHTML = "Uploading: " + percentComplete.toString() + '%';
    } else {
        progress.innerHTML = "Uploading";
    }
}

// When we're done uploading start the monitoring request
function uploadComplete(result) {
    var form = $('upload');
    var progress = $('progress');
    window.needToConfirm = false;

    if (result !== "failed") {
        var id = parseInt(result, 10);
        progress.innerHTML = "Processing";
        monitorProcessing(id);
    } else {
        fileError("Error uploading file");
    }
}

// Clear it with a space, so we don't have the table moving up and down the
// page when we upload a form
function clearProgress() {
    progress.innerHTML = "&nbsp;";
}

// If any errors occured
function fileError(msg) {
    var error = $('fileError');
    var progress = $('progress');
    var button = $('uploadFileButton');
    clearProgress();
    error.innerHTML = msg;
    button.disabled = false;
    window.needToConfirm = false;
}

// Delete an entry in the table including the header row and the data row
function deleteEntry(delElem) {
    var name;
    var id = parseInt(delElem.parentNode.firstElementChild.innerHTML, 10);

    for (var i = 0; i < delElem.parentNode.children.length; i++) {
        if (delElem.parentNode.children[i].className == "name") {
            name = delElem.parentNode.children[i].innerHTML;
            break;
        }
    }

    var result = confirm("Are you sure you want to delete \"" + name + "\"? "+
            "This cannot be undone.");

    if (result) {
        window.rpc.form_delete.on_result = function(r) {
            var row = delElem.parentNode.parentNode;
            var index = row.rowIndex;
            var nextRow = row.parentNode.rows[index + 1];

            row.parentNode.removeChild(row);
            nextRow.parentNode.removeChild(nextRow);
        };
        window.rpc.form_delete(id);
    }
}

// Request all of this users forms
function formGetAll() {
    window.rpc.form_getall.on_result = function(r) {
        var i;
        for (i = 0; i < r.length; ++i) {
            createEntry(r[i]["id"], r[i]["name"], r[i]["date"], r[i]["data"]);
        }
    };
    window.rpc.form_getall();
}

// Request the resulting data
function formGetOne(id) {
    window.rpc.form_getone.on_error = function(e) {
        fileError("Error downloading information");

        // Allow uploading again
        var button = $('uploadFileButton');
        button.disabled = false;
    };
    window.rpc.form_getone.on_result = function(r) {
        if (r.length == 1)
            createEntry(r[0]["id"], r[0]["name"], r[0]["date"], r[0]["data"]);

        // Allow uploading again
        var button = $('uploadFileButton');
        button.disabled = false;
    };
    window.rpc.form_getone(id);
}

// Insert a new entry into the table
function createEntry(id, name, date, formData) {
    var table = $("forms");

    var sId = document.createElement("span");
    sId.className = "id";
    sId.innerHTML = id;

	var sDel = document.createElement("span");
    sDel.className = "del";
    sDel.onclick = function() { deleteEntry(sDel) };
    sDel.innerHTML = "X";

	var sName = document.createElement("span");
    sName.className = "name";
    sName.innerHTML = name;

	var sDate = document.createElement("span");
    sDate.className = "date";
    sDate.innerHTML = "&mdash; " + date;

    // Data row
    var data = table.insertRow(1);
    data.className = "data";

    var dataCell = data.insertCell(0);
    dataCell.innerHTML = formData;

    // Header row
    var head = table.insertRow(1);
    head.className = "head";

    var headCell = head.insertCell(0);
    headCell.appendChild(sId);
    headCell.appendChild(sDel);
    headCell.appendChild(sName);
    headCell.appendChild(sDate);
}

/*
 * All pages
 */
function confirmExit() {
	if (window.needToConfirm)
        return "Are you sure you want to leave this page? You will lose data if you do."
}

function logoutOnclick() {
    window.rpc.account_logout.on_error = function(e) { };
    window.rpc.account_logout.on_result = function(r) { if (r) goHome(); };
    window.rpc.account_logout();
}

//  http("example.php",
//      function(data) { /* process  */ },
//      function(data) { /* error    */ },
//      function(evt)  { /* progress */ },
//      function(evt)  { /* canceled */ },
//      optionalData);
function http(url, complete, fail, progress, cancel, send) {
	var con;
	try       { con = new XMLHttpRequest(); }
	catch (e) { try { con = new ActiveXObject("Msxml2.XMLHTTP"); }
	catch (e) { try { con = new ActiveXObject("Microsoft.XMLHTTP"); }
	catch (e) { return; } } }

    if (typeof complete === "undefined")
        return; // Don't bother creating the request
    if (typeof fail === "undefined")
        fail = function() { };
    if (typeof progress === "undefined")
        progress = function() { };
    if (typeof cancel === "undefined")
        cancel = function() { };
    if (typeof send === "undefined")
        send = null;

    if (typeof con.addEventListener === "function") {
        con.addEventListener("progress", progress, false);
        con.addEventListener("load", function(evt) {
            if (evt.target.status === 200) {
                complete(evt.target.responseText);
            } else {
                fail(evt.target.responseText);
            }
        }, false);
        con.addEventListener("error", function(evt) {
            fail(evt.target.responseText);
        }, false);
        con.addEventListener("abort", cancel, false);
    } else {
        // See readyState values
        //   http://w3schools.com/ajax/ajax_xmlhttprequest_onreadystatechange.asp
        con.onreadystatechange = function() {
            if (con.readyState === 4) {
                if (con.status === 200) {
                    complete(con.responseText);
                } else {
                    fail(con.responseText);
                }
            }
        }
    }

	con.open("POST", url, true);
	con.send(send);
}

function goHome() {
    window.location.replace("/");
}

var $ = function(id) {
	return document.getElementById(id);
}

window.onload = function() {
    // Save on exit if needed
    window.needToConfirm = false;
    window.onbeforeunload = confirmExit;

    // JSON RPC setup
    window.rpc = new JsonRPC("/rpc", [
            "account_login",
            "account_logout",
            "account_create",
            "account_update",
            "account_delete",
            "form_process",
            "form_delete",
            "form_rename",
            "form_getall",
            "form_getone"
        ], []);

    // Whenever logged in
    if ($("logout") !== null) {
        var logoutLink = $("logout");
        logoutLink.onclick = logoutOnclick;
    }

    // The account page, logged out
    if ($("account") !== null) {
        var forgot = $("forgotlink");
        forgot.onmouseover = forgotMouseOver;
        forgot.onmouseout = forgotMouseOut;
        forgot.onclick = function() { return false; }

        var account = $("account");
        account.onsubmit = accountSubmit;

        var new_account = $("new_account");
        new_account.onsubmit = newAccountSubmit;
    }

    // The account page, logged in
    if ($("update_account") !== null) {
        var update_account = $("update_account");
        update_account.onsubmit = updateAccountSubmit;

        var delete_account = $("delete_account");
        delete_account.onclick = deleteAccount;
    }

    // The forms page
    if ($("upload") !== null) {
        var fileUpload = $("uploadFile");
        fileUpload.onchange = fileSelected;

        var uploadForm = $("upload");
        uploadForm.onsubmit = uploadFile;

        formGetAll();
    }
}
