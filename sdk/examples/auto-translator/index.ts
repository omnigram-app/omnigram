/// <reference path="../../omnigram.d.ts" />

/**
 * Omnigram Example Plugin: Auto Translator & Text Enhancer
 */

console.log(`[${omnigram.manifest.name}] Initialized v${omnigram.manifest.version}`);

// Show a welcome toast when the plugin loads
omnigram.ui.showToast(`Plugin "${omnigram.manifest.name}" is active!`);

// 1. Listen to incoming messages
omnigram.messages.on('message', (msg) => {
    if (!msg.isOutgoing) {
        console.log(`[AutoTranslator] New incoming message from ${msg.senderId}: ${msg.text}`);
    }
});

// 2. Intercept and format text before sending
omnigram.messages.onBeforeSend((text) => {
    // Example: Replace (c) with copyright symbol
    let modified = text.replace(/\(c\)/gi, '©');
    return modified;
});
