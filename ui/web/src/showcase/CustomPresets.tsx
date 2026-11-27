import { useRef, useState } from 'react';
import * as UI from '../library';

// Showcase only: custom presets saved in this browser, plus export and import of the shared preset file.
const storageKey = 'sj-v3-custom-presets';
export function loadCustomPresets(): UI.CustomPreset[] {
  try {
    const list = JSON.parse(localStorage.getItem(storageKey) || '[]');
    if (!Array.isArray(list)) return [];
    return list.flatMap(item => { const checked = item && typeof item === 'object' ? UI.checkCustomPreset(item) : undefined; return checked?.preset ? [checked.preset] : []; });
  } catch { return []; }
}
export function saveCustomPresets(list: UI.CustomPreset[]) { try { localStorage.setItem(storageKey, JSON.stringify(list)); } catch { /* Optional storage. */ } }
const sameName = (a: string, b: string) => a.trim().toLowerCase() === b.trim().toLowerCase();
export const sameChoices = (a: Partial<UI.PresetChoices>, b: Partial<UI.PresetChoices>) => JSON.stringify(Object.entries(a).sort()) === JSON.stringify(Object.entries(b).sort());
const fileName = (name: string) => `${name.trim().toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '') || 'custom'}.preset.json`;
const presetLabel = (id: string) => UI.presets.find(p => p.id === id)?.label ?? id;
const changedText = (n: number) => n === 0 ? 'no changed choices' : `${n} changed ${n === 1 ? 'choice' : 'choices'}`;

type Props = {
  customs: UI.CustomPreset[]; onCustomsChange: (list: UI.CustomPreset[]) => void;
  /** Applies a custom preset's base and choices and selects it. */ onSelect: (preset: UI.CustomPreset) => void;
  /** Marks which custom preset is selected without changing the current choices. */ onMark: (name: string | undefined) => void;
  selected?: UI.CustomPreset;
  base: UI.PresetId; overrides: Partial<UI.PresetChoices>;
};
type Message = { tone: 'success' | 'danger'; title: string; text?: string };

export function CustomPresetSettings({ customs, onCustomsChange, selected, onSelect, onMark, base, overrides }: Props) {
  const [name, setName] = useState(''); const [nameError, setNameError] = useState('');
  const [renaming, setRenaming] = useState<string | null>(null); const [renameError, setRenameError] = useState('');
  const [confirmDelete, setConfirmDelete] = useState(false);
  const [message, setMessage] = useState<Message | null>(null);
  const fileInput = useRef<HTMLInputElement>(null);
  const modified = !!selected && (selected.base !== base || !sameChoices(selected.choices, overrides));
  const taken = (candidate: string, except?: string) => customs.some(c => sameName(c.name, candidate) && !(except && sameName(c.name, except)));

  function saveNew() {
    const checked = UI.checkCustomPreset({ name, base, choices: overrides });
    if (!checked.preset) { setNameError(checked.error); return; }
    if (taken(checked.preset.name)) { setNameError(`A custom preset named "${checked.preset.name}" already exists.`); return; }
    onCustomsChange([...customs, checked.preset]); onMark(checked.preset.name);
    setName(''); setNameError(''); setMessage({ tone: 'success', title: `Saved ${checked.preset.name}` });
  }
  function rename() {
    if (!selected || renaming === null) return;
    const checked = UI.checkCustomPreset({ ...selected, name: renaming });
    if (!checked.preset) { setRenameError(checked.error); return; }
    if (taken(checked.preset.name, selected.name)) { setRenameError(`A custom preset named "${checked.preset.name}" already exists.`); return; }
    onCustomsChange(customs.map(c => c === selected ? checked.preset : c)); onMark(checked.preset.name);
    setRenaming(null); setRenameError(''); setMessage({ tone: 'success', title: `Renamed to ${checked.preset.name}` });
  }
  function update() {
    if (!selected) return;
    const checked = UI.checkCustomPreset({ name: selected.name, base, choices: overrides });
    if (!checked.preset) return;
    onCustomsChange(customs.map(c => c === selected ? checked.preset : c)); onMark(checked.preset.name);
    setMessage({ tone: 'success', title: `Updated ${selected.name}` });
  }
  function remove() {
    if (!selected) return;
    onCustomsChange(customs.filter(c => c !== selected)); onMark(undefined);
    setConfirmDelete(false); setMessage({ tone: 'success', title: `Deleted ${selected.name}`, text: 'The current choices stay as they are.' });
  }
  function exportFile() {
    if (!selected) return;
    const url = URL.createObjectURL(new Blob([UI.customPresetFile(selected)], { type: 'application/json' }));
    const a = document.createElement('a'); a.href = url; a.download = fileName(selected.name); a.click();
    setTimeout(() => URL.revokeObjectURL(url), 1000);
  }
  // Import adds the preset and selects it. A preset with the same name is replaced, so a file moved between machines updates it.
  async function importFile(file: File | undefined) {
    if (!file) return;
    let text = '';
    try { text = await file.text(); } catch { setMessage({ tone: 'danger', title: 'Preset file not imported', text: 'The file could not be read.' }); return; }
    const parsed = UI.parseCustomPresetFile(text);
    if (!parsed.preset) { setMessage({ tone: 'danger', title: 'Preset file not imported', text: parsed.error }); return; }
    const replaced = customs.some(c => sameName(c.name, parsed.preset.name));
    onCustomsChange([...customs.filter(c => !sameName(c.name, parsed.preset.name)), parsed.preset]); onSelect(parsed.preset);
    setMessage({ tone: 'success', title: `${replaced ? 'Replaced' : 'Imported'} ${parsed.preset.name}`, text: `Based on ${presetLabel(parsed.preset.base)}, ${changedText(Object.keys(parsed.preset.choices).length)}.` });
  }

  return <>
    {selected && <UI.SettingsRow label={selected.name} description={`Based on ${presetLabel(selected.base)}, ${changedText(Object.keys(selected.choices).length)}.${modified ? ' The current choices differ.' : ''}`}>
      {renaming !== null ? <div className="custom-preset-controls">
        <UI.TextInput label="New name" value={renaming} error={renameError} maxLength={60} onChange={e => { setRenaming(e.target.value); setRenameError(''); }} onKeyDown={e => { if (e.key === 'Enter') rename(); if (e.key === 'Escape') { e.preventDefault(); e.stopPropagation(); setRenaming(null); } }} autoFocus />
        <UI.Button variant="primary" onClick={rename}>Rename</UI.Button><UI.Button onClick={() => { setRenaming(null); setRenameError(''); }}>Cancel</UI.Button>
      </div> : confirmDelete ? <div className="custom-preset-controls">
        <span>Delete {selected.name}?</span><UI.Button variant="danger" onClick={remove}>Delete</UI.Button><UI.Button onClick={() => setConfirmDelete(false)}>Cancel</UI.Button>
      </div> : <div className="custom-preset-controls">
        <UI.Button disabled={!modified} onClick={update}>Update</UI.Button>
        <UI.Button onClick={() => { setRenaming(selected.name); setConfirmDelete(false); }}>Rename</UI.Button>
        <UI.Button onClick={exportFile}>Export file</UI.Button>
        <UI.Button onClick={() => setConfirmDelete(true)}>Delete</UI.Button>
      </div>}
    </UI.SettingsRow>}
    <UI.SettingsRow label="Save as custom preset" description="Name the current preset and choices to pick them again. Saved in this browser.">
      <div className="custom-preset-controls">
        <UI.TextInput label="Custom preset name" placeholder="e.g. My editor" value={name} error={nameError} maxLength={60} onChange={e => { setName(e.target.value); setNameError(''); }} onKeyDown={e => { if (e.key === 'Enter') saveNew(); }} />
        <UI.Button onClick={saveNew}>Save</UI.Button>
      </div>
    </UI.SettingsRow>
    <UI.SettingsRow label="Import preset file" description="Add a custom preset from a preset file exported on web or native.">
      <input ref={fileInput} type="file" accept=".json,application/json" hidden aria-label="Preset file" onChange={e => { void importFile(e.target.files?.[0]); e.target.value = ''; }} />
      <UI.Button onClick={() => fileInput.current?.click()}>Import file</UI.Button>
    </UI.SettingsRow>
    {message && <UI.Alert tone={message.tone} title={message.title} action={<UI.IconButton icon="close" label="Dismiss" variant="quiet" onClick={() => setMessage(null)} />}>{message.text}</UI.Alert>}
  </>;
}
