
let App = {};

init();

function init() {
    let buttons = window.document.getElementsByClassName("button");
    for (button of buttons) {
        button.addEventListener("pointerdown", onPress);
    }

    App.entry = () => window.document.getElementById("entry");
    App.stack = () => window.document.getElementById("stack");
}

function onPress(event) {
    if (event.button == 0) {

        let text = event.target.textContent.trim();

        if (isDigit(text)) {
            let entry = window.document.getElementById("entry");
            entry.textContent += text;
        }
        else if (text == "clear") {
            App.entry().textContent = "";
        }
        else if (text == "push") {
            push(App.entry().textContent);
            App.entry().textContent = "";
        }
        else if (text == "drop") {
            pop();
        }
        else if (text == "swap") {
            let entries = App.stack().children;
            if (entries.length >= 2) {
                let entry = entries[entries.length - 2];
                entry.remove();
                App.stack().appendChild(entry);
            }
        }
        else if (text == "e") {
            push("2.7182818284");
        }
        else if (text == "π") {
            push("3.1415926535");
        }
        else if (text == "+") {
            if (count() >= 2) {
                let n2 = pop();
                let n1 = pop();
                push(n1 + n2);
            }
        }
        else if (text == "-") {
            if (count() >= 2) {
                let n2 = pop();
                let n1 = pop();
                push(n1 - n2);
            }
        }
        else if (text == "*") {
            if (count() >= 2) {
                let n2 = pop();
                let n1 = pop();
                push(n1 * n2);
            }
        }
        else if (text == "/") {
            if (count() >= 2) {
                let n2 = pop();
                let n1 = pop();
                push(n1 / n2);
            }
        }
        else if (text == "^") {
            if (count() >= 2) {
                let n2 = pop();
                let n1 = pop();
                push(Math.pow(n1, n2));
            }
        }
    }
}

function push(text) {
    let element = document.createElement("div");
    element.textContent = text;
    App.stack().appendChild(element);
}

function pop() {
    let element = App.stack().lastChild;
    element.remove();
    return +(element.textContent);
}

function count() {
    return App.stack().children.length;
}

function isDigit(text) {
    if (text.length != 1) return false;
    return text.charCodeAt() >= '0'.charCodeAt() && text.charCodeAt() <= '9'.charCodeAt() || text == '.';
}