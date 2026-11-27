import { useState } from 'react';
import * as UI from '../library';
import { starter, starterKinds } from './starter';
import { zip } from './zip';

type Platform = typeof starterKinds[number]['platform'];
const mainFile: Record<Platform, string> = { web: 'src/App.tsx', native: 'main.cpp' };

// Showcase only: Export writes a web or native starter project for the chosen preset and changed choices, as one zip.
export function ExportDialog({ open, onClose, preset, overrides, customName }: { open: boolean; onClose: () => void; preset: UI.PresetId; overrides: Partial<UI.PresetChoices>; customName?: string }) {
  const [platform, setPlatform] = useState<Platform>('web');
  const [shown, setShown] = useState<Record<Platform, string>>(mainFile);
  const result = starter(platform, preset, overrides);
  const files = result.files ?? [];
  const file = files.find(f => f.path === shown[platform]) ?? files[0];
  const presetLabel = UI.presets.find(p => p.id === preset)?.label ?? preset;
  const changed = Object.keys(UI.presetOverrides(preset, overrides)).length;
  function download() {
    if (!files.length) return;
    const url = URL.createObjectURL(zip(files.map(f => ({ path: `my-app/${f.path}`, text: f.text }))));
    const a = document.createElement('a'); a.href = url; a.download = `my-app-${platform}-starter.zip`; a.click();
    setTimeout(() => URL.revokeObjectURL(url), 1000);
  }
  return <UI.Dialog open={open} onClose={onClose} title="Export starter" size="large" actions={<><UI.Button onClick={onClose}>Close</UI.Button><UI.Button variant="primary" disabled={!files.length} onClick={download}>Download zip</UI.Button></>}>
    <div className="export-body">
      <p className="showcase-only-note">Showcase only. The starter uses {customName ? <>the custom preset {customName}: </> : null}the {presetLabel} preset{changed ? ` with ${changed} changed ${changed === 1 ? 'choice' : 'choices'}` : ''} and installs the library from the v{UI.systemVersion} release tag. It copies no components or styles.</p>
      <div className="export-options">
        <UI.RadioGroup label="Starter" value={platform} onChange={v => setPlatform(v as Platform)} options={starterKinds.map(k => ({ value: k.platform, label: `${k.label}: ${k.description}` }))} />
        {file && <UI.Select label="Preview file" value={file.path} onChange={e => setShown(s => ({ ...s, [platform]: e.target.value }))}>{files.map(f => <option key={f.path} value={f.path}>{f.path}</option>)}</UI.Select>}
      </div>
      {result.unsupported ? <UI.Alert tone="info" title="No native starter for this preset">{result.unsupported}</UI.Alert>
        : file && <UI.CodeBlock language={file.path} code={file.text} />}
    </div>
  </UI.Dialog>;
}
