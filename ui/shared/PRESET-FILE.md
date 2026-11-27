# Custom preset file

A custom preset is a name, a base Preset and only the choices that differ from that base. Web and native save, export and import the same file.

```json
{
  "format": "stack-junkie-preset",
  "version": 1,
  "name": "My desktop setup",
  "base": "desktop-app",
  "choices": {
    "sidebar": "tree",
    "theme": "light"
  }
}
```

## Fields

| Field | Rule |
|---|---|
| `format` | Always `"stack-junkie-preset"`. |
| `version` | Always `1`. Another version is rejected. |
| `name` | 1 to 60 characters after trimming. No line breaks or control characters. |
| `base` | A preset `id` from `presets.json`: `web-app`, `desktop-app`, `document-editor` or `website`. |
| `choices` | An object. Each key is a choice name from `presets.json`, and each value one of that choice's `options`, with the same JSON type (`true`, not `"true"`). May be empty. |

No other fields are allowed.

## Reading

- Any broken rule rejects the whole file with one plain message. Nothing is applied or saved.
- A choice equal to the base's value is accepted and dropped.
- A leading byte order mark is ignored.

## Writing

- Fields in the order above, two-space indent, UTF-8, a final newline.
- `choices` holds only the choices that differ from the base, in `presets.json` order.
- Web (`presetFileText`) and native (`CustomPreset::toFile`) write byte-identical text for the same preset.

## Where the rules live

- `preset-file.mjs`: the rules for web and the test script.
- `../native/src/presets.cpp` (`CustomPreset`): the same rules in C++.
- `presets.json`: the allowed base ids, choice names and values for both.
- `npm run test:presets` (in `ui/web`) checks a web to native round trip and that bad files are rejected on both sides.
