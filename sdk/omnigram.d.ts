/**
 * Omnigram TypeScript Plugin SDK
 * Definitions for creating plugins in Omnigram.
 */

export interface PluginManifest {
    id: string;
    name: string;
    version: string;
    description: string;
    author: string;
    main: string;
    enabled?: boolean;
    /** Whether this plugin should have its own dedicated root section in main Settings */
    standaloneSettingsTab?: boolean;
}

export interface ToastOptions {
    durationMs?: number;
}

export interface MessageEvent {
    id: number;
    peerId: number;
    senderId: number;
    text: string;
    date: number;
    isOutgoing: boolean;
}

export type SettingItemType = 'boolean' | 'string' | 'select' | 'button';

export interface SettingSelectOption {
    label: string;
    value: string;
}

export interface SettingItem {
    id: string;
    type: SettingItemType;
    title: string;
    description?: string;
    default?: any;
    options?: SettingSelectOption[]; // For 'select' type
    onClick?: () => void; // For 'button' type
}

export interface SettingsSectionOptions {
    title: string;
    icon?: string;
    standaloneTab?: boolean;
    items: SettingItem[];
}

export interface OmnigramUI {
    /** Show a floating toast notification in the application */
    showToast(text: string, options?: ToastOptions): void;
    /** Open Omnigram Settings */
    openSettings(): void;
    /** Register native settings section (shown via gear icon or standalone tab) */
    registerSettings(options: SettingsSectionOptions): void;
}

export interface OmnigramMessages {
    /** Listen to incoming or outgoing messages */
    on(event: 'message', listener: (msg: MessageEvent) => void): void;
    /** Intercept and optionally modify message text before sending */
    onBeforeSend(listener: (text: string) => string | void): void;
}

export interface OmnigramStorage {
    /** Read stored data by key */
    get<T = any>(key: string, defaultValue?: T): T;
    /** Store data by key */
    set<T = any>(key: string, value: T): void;
    /** Remove stored key */
    delete(key: string): void;
}

export interface OmnigramGlobal {
    manifest: PluginManifest;
    ui: OmnigramUI;
    messages: OmnigramMessages;
    storage: OmnigramStorage;
    version: string;
}

declare global {
    const omnigram: OmnigramGlobal;
}

export {};
