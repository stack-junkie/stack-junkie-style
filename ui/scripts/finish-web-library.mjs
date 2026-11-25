import { mkdir, writeFile, cp } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import path from 'node:path';
const ui = fileURLToPath(new URL('../', import.meta.url));
const declarations = path.join(ui, 'web/dist-types/web/src/library');
await mkdir(declarations, { recursive: true });
await writeFile(path.join(declarations, 'styles.css.d.ts'), 'export {};\n');
await cp(path.join(ui, 'LICENSE'), path.join(ui, 'web/dist-library/LICENSE'));
await cp(path.join(ui, 'shared/licenses'), path.join(ui, 'web/dist-library/licenses'), { recursive: true });
for (const font of ['Manrope', 'Lora', 'FiraCode']) {
  await cp(path.join(ui, `shared/fonts/${font}-OFL.txt`), path.join(ui, `web/dist-library/licenses/${font}-OFL.txt`));
}
