import { useId } from 'react';
import { directions, useTheme } from './theme';
import tokens from '../../../shared/tokens.json';
import selectedMark from '../../../shared/brand/sj-transparent-mask.png';

export function BrandMark({ size = tokens.density.brandMarkSize }: { size?: number }) {
  const { direction, mode } = useTheme();
  const palette = directions[direction][mode];
  const filterId = useId();
  const channel = (color: string, offset: number) => parseInt(color.slice(offset, offset + 2), 16) / 255;
  const values = (offset: number) => `${channel(palette.text, offset)} ${channel(palette.accent, offset)}`;
  return <svg className="sj-brand-mark" width={size} height={size} viewBox="0 0 100 100" aria-hidden="true" focusable="false">
      <defs><filter id={filterId} colorInterpolationFilters="sRGB">
        {/* Black S and white J are recolored from the shared theme, preserving alpha. */}
        <feComponentTransfer>
          <feFuncR type="table" tableValues={values(1)} />
          <feFuncG type="table" tableValues={values(3)} />
          <feFuncB type="table" tableValues={values(5)} />
        </feComponentTransfer>
      </filter></defs>
      <image href={selectedMark} width="100" height="100" filter={`url(#${filterId})`} />
    </svg>;
}
