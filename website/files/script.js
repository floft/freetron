/*
 * Account page
 */

function getPos(elem) {
    var x = 0;
    var y = 0;

    while (true) {
        x += elem.offsetLeft;
        y += elem.offsetTop;

        if (elem.offsetParent === null)
            break;

        elem = elem.offsetParent;
    }

    return [x, y];
}

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
    /*var link = $("forgotlink");
    var pos = getPos(link);
    box.style.left = pos[0] + "px";
    box.style.top = (pos[1]+20) + "px";*/
    box.style.display = "block";
}

function forgotMouseOut() {
    var box  = $("forgotmsg");
    box.style.display = "none";
}

function forgotOnclick() {
    var link = $("forgotlink");
    var box  = $("forgotmsg");

    if (box.style.display === "" || box.style.display === "none") {
        box.style.left = link.offsetLeft;
        box.style.top = link.offsetTop;
        box.style.display = "block";
    } else {
        box.style.display = "none";
    }

    return false;
}

function deleteAccount() {
    var result = confirm("Are you sure you want to delete your account? All of "+
            "your data will be permanently removed.");

    if (result) {
        var confirmationCode = parseInt($("confirm").value, 10);
        window.rpc.account_delete.on_error = function(e) { };
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
        var fileSize = 0;

        if (file.size > 1024 * 1024)
            fileSize = (Math.round(file.size * 100 / (1024 * 1024)) / 100).toString() + 'MB';
        else
            fileSize = (Math.round(file.size * 100 / 1024) / 100).toString() + 'KB';

        if (isPdf(file.type, file.name)) {
            error.style.display = "none";
        } else {
            error.innerHTML = "Must be PDF";
            error.style.display = "inline";
        }

        if ($('fileName'))
        {
            $('fileName').innerHTML = 'Name: ' + file.name;
            $('fileSize').innerHTML = 'Size: ' + fileSize;
            $('fileType').innerHTML = 'Type: ' + file.type;
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

    if (file && isPdf(file.type, file.name) && validKey(key)) {
        var fd;

        // If this doesn't work at some point, look at
        // https://developer.mozilla.org/en-US/docs/Web/Guide/HTML/Forms/Sending_forms_through_JavaScript
        var form = $('upload');

        if (typeof form.getFormData  === "function") {
            fd = form.getFormData();
        } else {
            fd = new FormData(form);
        }

        http("/upload/" + key,
        uploadComplete, function(data) {
            fileError("Error uploading file");
        }, uploadProgress, function(evt) {
            fileError("Canceled");
        }, fd);

        progress.style.display = "inline";
        error.style.display = "none";
        button.disabled = true;
        window.needToConfirm = true;
    }

    return false;
}

// Create a request to see the processing status
function monitorProcessing(id) {
    http("/process/" + id,
    processComplete, function(data) {
        fileError("Error processing file");
    }, processProgress, function(evt) {
        fileError("Processing canceled");
    });
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

function processProgress(evt) {
    var progress = $('progress');
    var result = evt.target.responseText;

    if (evt.lengthComputable) {
        var percentComplete = Math.round(evt.loaded * 100 / evt.total);
        progress.innerHTML = "Processing: " + percentComplete.toString() + '%';
    } else {
        var lines = result.split("\n");
        var percentComplete = lines[lines.length - 2];
        progress.innerHTML = "Processing: " + percentComplete + '%';
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
        var error = $('fileError');
        progress.innerHTML = "";
        error.innerHTML = "Error uploading file";
        error.style.display = "inline";
    }
}

// Reset the form when it's done processing and request
// the results
function processComplete(result) {
    var lines = result.split("\n");
    var lastLine = lines[lines.length - 2];
    var form = $('upload');
    var progress = $('progress');

    if (lastLine !== "failed") {
        // Processing bar
        form.reset();
        progress.innerHTML = "Done";

        var id = parseInt(lastLine, 10);
        formGetOne(id);

        setTimeout(function() {
            if (progress.innerHTML === "Done")
                progress.innerHTML = "";
        }, 3000);
    } else {
        var error = $('fileError');
        progress.innerHTML = "";
        error.innerHTML = "Error processing file";
        error.style.display = "inline";
    }
}

// If any errors occured
function fileError(msg) {
    var error = $('fileError');
    var progress = $('progress');
    var button = $('uploadFileButton');
    progress.innerHTML = "";
    error.innerHTML = msg;
    error.style.display = "inline";
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
function vertScroll() {
    return document.body.scrollHeight > document.body.clientHeight;

    if (window.innerHeight)
        return document.body.offsetHeight > window.innerHeight;
    else
        return document.documentElement.scrollHeight > document.documentElement.offsetHeight ||
        document.body.scrollHeight > document.body.offsetHeight;
}

function resize() {
    var show = vertScroll();
    var elem = $("footer_color");

    elem.style.display = (show)?"block":"none";
}

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
    // Show/hide the footer color bar at bottom only if there is a vertical
    // scroll bar, otherwise it looks odd
    //window.onresize = resize;
    //resize();

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
        //forgot.onclick = forgotOnclick;

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
