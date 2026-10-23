(() => {
    "use strict";

    const SEND_INTERVAL_MS = 1;

    const GROUPS = {
        frontHeadLight: "Headlights",
        frontTurnSignal: "Turn signals",
        cabinLight: "Cabin",
        backBreakLight: "Brake lights"
    };

    // RGB values in 0xRRGGBB format, matching the ESP32.
    const colors = {
        frontHeadLight: 0xFFFFFF,
        frontTurnSignal: 0xFFA500,
        cabinLight: 0xFFFFFF,
        backBreakLight: 0xFF0000
    };

    const pickerCanvas = document.querySelector("#picker-canvas");
    const hueCanvas = document.querySelector("#hue-canvas");

    if (!pickerCanvas || !hueCanvas) {
        console.error("Color picker canvas not found.");
        return;
    }

    const pickerCtx = pickerCanvas.getContext("2d");
    const hueCtx = hueCanvas.getContext("2d");

    const selectedTitle = document.querySelector("#selected-title");
    const selectedHex = document.querySelector("#selected-hex");
    const preview = document.querySelector("#color-preview");
    const status = document.querySelector("#status");


    let activeGroup = "frontHeadLight";

    let hue = 0;
    let sat = 0;
    let val = 100;

    let pickerX = 0;
    let pickerY = 0;
    let hueX = 0;

    let isDragging = false;
    let activeCanvas = null;
    let activeIsHue = false;

    let lastSentAt = 0;
    let sendTimer = null;
    let requestInFlight = false;
    let pendingPayload = null;

    // Convert RGB array to #RRGGBB.
    function rgbToHex(rgb) {
        return "#" + rgb
            .map(value =>
                Math.max(0, Math.min(255, Math.round(value)))
                    .toString(16)
                    .padStart(2, "0")
            )
            .join("")
            .toUpperCase();
    }

    // Convert #RRGGBB to a numeric 0xRRGGBB value.
    function hexToNumber(hex) {
        return Number.parseInt(hex.replace(/^#/, ""), 16) >>> 0;
    }

    // Convert HSV to RGB.
    function hsvToRgb(h, s, v) {
        h = ((h % 360) + 360) % 360;
        s = Math.max(0, Math.min(100, s)) / 100;
        v = Math.max(0, Math.min(100, v)) / 100;

        const c = v * s;
        const x = c * (1 - Math.abs((h / 60) % 2 - 1));
        const m = v - c;

        let r, g, b;

        if (h < 60) {
            [r, g, b] = [c, x, 0];
        } else if (h < 120) {
            [r, g, b] = [x, c, 0];
        } else if (h < 180) {
            [r, g, b] = [0, c, x];
        } else if (h < 240) {
            [r, g, b] = [0, x, c];
        } else if (h < 300) {
            [r, g, b] = [x, 0, c];
        } else {
            [r, g, b] = [c, 0, x];
        }

        return [
            Math.round((r + m) * 255),
            Math.round((g + m) * 255),
            Math.round((b + m) * 255)
        ];
    }

    // Convert RGB to HSV.
    function rgbToHsv(r, g, b) {
        r /= 255;
        g /= 255;
        b /= 255;

        const max = Math.max(r, g, b);
        const min = Math.min(r, g, b);
        const d = max - min;

        let h = 0;

        if (d !== 0) {
            if (max === r) {
                h = 60 * (((g - b) / d) % 6);
            } else if (max === g) {
                h = 60 * ((b - r) / d + 2);
            } else {
                h = 60 * ((r - g) / d + 4);
            }
        }

        if (h < 0) {
            h += 360;
        }

        return {
            h,
            s: max === 0 ? 0 : (d / max) * 100,
            v: max * 100
        };
    }

    function currentRgb() {
        return hsvToRgb(hue, sat, val);
    }

    function currentHex() {
        return rgbToHex(currentRgb());
    }

    function setStatus(message, kind = "") {
        if (!status) return;

        status.textContent = message;
        status.className = "status" + (kind ? " " + kind : "");
    }

    // Draw the saturation/value picker.
    function drawPicker() {
        pickerCtx.fillStyle = `hsl(${hue}, 100%, 50%)`;
        pickerCtx.fillRect(
            0, 0, pickerCanvas.width, pickerCanvas.height
        );

        const white = pickerCtx.createLinearGradient(
            0, 0, pickerCanvas.width, 0
        );

        white.addColorStop(0, "rgba(255,255,255,1)");
        white.addColorStop(1, "rgba(255,255,255,0)");

        pickerCtx.fillStyle = white;
        pickerCtx.fillRect(
            0, 0, pickerCanvas.width, pickerCanvas.height
        );

        const black = pickerCtx.createLinearGradient(
            0, 0, 0, pickerCanvas.height
        );

        black.addColorStop(0, "rgba(0,0,0,0)");
        black.addColorStop(1, "rgba(0,0,0,1)");

        pickerCtx.fillStyle = black;
        pickerCtx.fillRect(
            0, 0, pickerCanvas.width, pickerCanvas.height
        );

        pickerX = (sat / 100) * pickerCanvas.width;
        pickerY = ((100 - val) / 100) * pickerCanvas.height;

        pickerCtx.beginPath();
        pickerCtx.arc(pickerX, pickerY, 6, 0, Math.PI * 2);
        pickerCtx.lineWidth = 1;
        pickerCtx.strokeStyle = "#000";
        pickerCtx.stroke();

        pickerCtx.beginPath();
        pickerCtx.arc(pickerX, pickerY, 4, 0, Math.PI * 2);
        pickerCtx.lineWidth = 2;
        pickerCtx.strokeStyle = "#fff";
        pickerCtx.stroke();
    }

    // Draw the hue slider.
    function drawHue() {
        const gradient = hueCtx.createLinearGradient(
            0, 0, hueCanvas.width, 0
        );

        for (let i = 0; i <= 360; i += 10) {
            gradient.addColorStop(
                i / 360,
                `hsl(${i}, 100%, 50%)`
            );
        }

        hueCtx.fillStyle = gradient;
        hueCtx.fillRect(0, 0, hueCanvas.width, hueCanvas.height);

        hueX = (hue / 360) * hueCanvas.width;

        hueCtx.strokeStyle = "#000";
        hueCtx.lineWidth = 1;
        hueCtx.strokeRect(
            hueX - 3, 0, 6, hueCanvas.height
        );

        hueCtx.strokeStyle = "#fff";
        hueCtx.lineWidth = 2;
        hueCtx.strokeRect(
            hueX - 2, 1, 4, hueCanvas.height - 2
        );
    }

    // Update the page to reflect the selected color.
    function render() {
        const rgb = currentRgb();
        const hex = rgbToHex(rgb);

        if (selectedTitle) {
            selectedTitle.textContent = GROUPS[activeGroup];
        }

        if (selectedHex) {
            selectedHex.textContent = hex;
        }

        if (preview) {
            preview.style.backgroundColor = hex;
        }

        for (const [key, value] of Object.entries(colors)) {
            const itemHex =
                "#" + (value >>> 0).toString(16)
                    .padStart(6, "0")
                    .toUpperCase();

            const swatch = document.querySelector(`#swatch-${key}`);
            const hexLabel = document.querySelector(`#hex-${key}`);

            if (swatch) {
                swatch.style.backgroundColor = itemHex;
            }

            if (hexLabel) {
                hexLabel.textContent = itemHex;
            }
        }

        document.querySelectorAll(".group").forEach(button => {
            button.classList.toggle(
                "active",
                button.dataset.group === activeGroup
            );
        });

        drawPicker();
        drawHue();
    }

    // Snapshot of all colors, using numeric 0xRRGGBB values.
    function createPayload() {
        return {
            frontHeadLight: colors.frontHeadLight >>> 0,
            frontTurnSignal: colors.frontTurnSignal >>> 0,
            cabinLight: colors.cabinLight >>> 0,
            backBreakLight: colors.backBreakLight >>> 0
        };
    }

    // Schedule updates while dragging, limited to 100 ms intervals.
    function scheduleSend() {
        pendingPayload = createPayload();

        if (!isDragging || sendTimer !== null || requestInFlight) {
            return;
        }

        const wait = Math.max(
            0,
            SEND_INTERVAL_MS - (Date.now() - lastSentAt)
        );

        sendTimer = setTimeout(() => {
            sendTimer = null;
            sendColors();
        }, wait);
    }

    // Send the latest colors to the ESP32.
    async function sendColors() {
        if (!pendingPayload || requestInFlight) {
            return;
        }

        const payload = pendingPayload;
        pendingPayload = null;

        requestInFlight = true;
        lastSentAt = Date.now();

        console.log("POST /colors payload:", JSON.stringify(payload));

        try {
            const response = await fetch("/colors", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json"
                },
                body: JSON.stringify(payload),
                cache: "no-store"
            });

            const responseText = await response.text();

            console.log(
                "POST /colors response:",
                response.status,
                responseText
            );

            if (!response.ok) {
                throw new Error(
                    `HTTP ${response.status}: ${responseText}`
                );
            }

            setStatus("Connected · colors updated", "success");
        } catch (error) {
            console.error("Could not send colors to ESP32:", error);
            setStatus("Could not send colors to the car", "error");
        } finally {
            requestInFlight = false;

            if (pendingPayload) {
                if (isDragging) {
                    scheduleSend();
                } else {
                    sendColors();
                }
            }
        }
    }

    function updateActiveColor() {
        colors[activeGroup] = hexToNumber(currentHex());
        render();
        scheduleSend();
    }

    // Update HSV based on pointer position.
    function setPickerFromPointer(canvas, event, isHue) {
        const rect = canvas.getBoundingClientRect();

        if (!rect.width || !rect.height) {
            return;
        }

        const x = Math.max(
            0,
            Math.min(1, (event.clientX - rect.left) / rect.width)
        );

        const y = Math.max(
            0,
            Math.min(1, (event.clientY - rect.top) / rect.height)
        );

        if (isHue) {
            hue = x * 360;
        } else {
            sat = x * 100;
            val = (1 - y) * 100;
        }

        updateActiveColor();
    }

    function beginDrag(canvas, event, isHue) {
        event.preventDefault();

        isDragging = true;
        activeCanvas = canvas;
        activeIsHue = isHue;

        if (canvas.setPointerCapture) {
            try {
                canvas.setPointerCapture(event.pointerId);
            } catch (error) {
                // Global pointer events still handle dragging.
            }
        }

        setPickerFromPointer(canvas, event, isHue);
    }

    function endDrag() {
        if (!isDragging) {
            return;
        }

        isDragging = false;
        activeCanvas = null;

        if (sendTimer !== null) {
            clearTimeout(sendTimer);
            sendTimer = null;
        }

        // Send the final selection immediately.
        if (pendingPayload && !requestInFlight) {
            sendColors();
        }
    }

    // Global movement means dragging keeps working outside the canvas.
    window.addEventListener("pointermove", event => {
        if (!isDragging || !activeCanvas) {
            return;
        }

        if (event.buttons === 0) {
            endDrag();
            return;
        }

        setPickerFromPointer(
            activeCanvas,
            event,
            activeIsHue
        );
    });

    window.addEventListener("pointerup", endDrag);
    window.addEventListener("pointercancel", endDrag);

    pickerCanvas.addEventListener("pointerdown", event => {
        beginDrag(pickerCanvas, event, false);
    });

    hueCanvas.addEventListener("pointerdown", event => {
        beginDrag(hueCanvas, event, true);
    });

    // Switch between color groups.
    document.querySelectorAll(".group").forEach(button => {
        button.addEventListener("click", () => {
            const group = button.dataset.group;

            if (!Object.prototype.hasOwnProperty.call(colors, group)) {
                return;
            }

            activeGroup = group;

            const color = colors[activeGroup] >>> 0;

            const rgb = [
                (color >> 16) & 255,
                (color >> 8) & 255,
                color & 255
            ];

            const hsv = rgbToHsv(...rgb);

            hue = hsv.h;
            sat = hsv.s;
            val = hsv.v;

            render();
            setStatus("Ready");
        });
    });

    document.querySelector("#apply-button")?.addEventListener("click", applyColors);
    document.querySelector("#restart-button")?.addEventListener("click", restartESP32);
    document.querySelector("#defaults-button")?.addEventListener("click", loadDefaults);

    async function getCurrentColors() {
        try {
            const response = await fetch("/colors", {
                method: "GET",
                cache: "no-store"
            });

            if (!response.ok) {
                throw new Error(`HTTP ${response.status}`);
            }

            const currentColors = await response.json();

            Object.assign(colors, currentColors);

            // Update the picker to match the currently selected group.
            const color = colors[activeGroup] >>> 0;

            const rgb = [
                (color >> 16) & 255,
                (color >> 8) & 255,
                color & 255
            ];

            const hsv = rgbToHsv(...rgb);

            hue = hsv.h;
            sat = hsv.s;
            val = hsv.v;

            render();
            setStatus("Connected · colors loaded", "success");

            console.log("Current colors loaded:", currentColors);
        } catch (error) {
            console.error("Could not load colors from ESP32:", error);
            render();
            setStatus("Could not load colors from the car", "error");
        }
    }

    async function applyColors() {
        try {
            const response = await fetch("/colors-save", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json"
                },
                body: JSON.stringify(createPayload()),
                cache: "no-store"
            });

            if (!response.ok) {
                throw new Error(`HTTP ${response.status}`);
            }

            setStatus("Colors saved", "success");
        } catch (error) {
            console.error("Could not save colors:", error);
            setStatus("Could not save colors", "error");
        }
    }

    async function restartESP32() {
        try {
            const response = await fetch("/restart", {
                method: "GET",
                cache: "no-store"
            });

            if (!response.ok) {
                throw new Error(`HTTP ${response.status}`);
            }

            setStatus("ESP32 restarting...", "success");
        } catch (error) {
            // The ESP32 may disconnect before responding during reboot.
            console.error("Restart request failed:", error);
            setStatus("Restart requested or connection lost");
        }
    }

    async function loadDefaults() {
        try {
            const response = await fetch("/colors-default", {
                method: "GET",
                cache: "no-store"
            });

            if (!response.ok) {
                throw new Error(`HTTP ${response.status}`);
            }

            const defaultColors = await response.json();
            Object.assign(colors, defaultColors);

            const color = colors[activeGroup] >>> 0;
            const rgb = [
                (color >> 16) & 255,
                (color >> 8) & 255,
                color & 255
            ];

            const hsv = rgbToHsv(...rgb);
            hue = hsv.h;
            sat = hsv.s;
            val = hsv.v;

            render();
            setStatus("Default colors loaded", "success");
        } catch (error) {
            console.error("Could not load default colors:", error);
            setStatus("Could not load default colors", "error");
        }
    }

    // Load current colors from the ESP32 when the page opens.
    getCurrentColors();
})();