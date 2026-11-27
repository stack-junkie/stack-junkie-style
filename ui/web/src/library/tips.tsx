import { createContext, useContext, useEffect, useLayoutEffect, useRef, useState } from 'react';
import { createPortal } from 'react-dom';

// Tooltips sit against the element they describe: to the right of Rail items, below everything else.
// Any element inside a theme gets one from `data-sj-tip="Text"`, and `data-sj-tip-side="right"` moves it beside the element.
// The outermost ThemeProvider draws them in a layer on the page body, carrying the theme's colors, so a zoomed or
// scrolled area never moves a tooltip away from its element.
export type TipSide = 'below' | 'right';
/** The attributes that give an element a tooltip. Pass the element's accessible name as text. */
export function tipProps(text: string | undefined, side: TipSide = 'below') {
  return text ? { 'data-sj-tip': text, ...(side === 'right' ? { 'data-sj-tip-side': 'right' } : {}) } : {};
}

const Layered = createContext(false);
export const useTipLayered = () => useContext(Layered);
export const TipLayerScope = Layered.Provider;

// A tooltip inside a modal dialog renders in that dialog, which sits above the rest of the page.
type Shown = { text: string; side: TipSide; rect: DOMRect; host: Element };
const delay = 400;

export function TipLayer() {
  const anchor = useRef<HTMLSpanElement>(null);
  const tipRef = useRef<HTMLDivElement>(null);
  const [tip, setTip] = useState<Shown | null>(null);
  const [offset, setOffset] = useState(0);
  const [themeStyle, setThemeStyle] = useState('');
  useEffect(() => {
    const root = anchor.current?.parentElement;
    if (!root) return;
    let timer: ReturnType<typeof setTimeout> | undefined; let current: Element | null = null;
    const find = (target: EventTarget | null) => target instanceof Element ? target.closest('[data-sj-tip]') : null;
    function hide() { clearTimeout(timer); current = null; setTip(null); }
    function show(element: Element, wait: number) {
      clearTimeout(timer); current = element;
      timer = setTimeout(() => {
        if (!element.isConnected || element.matches(':disabled')) return;
        setThemeStyle((root!.getAttribute('style') ?? '').replace(/(^|;)\s*zoom:[^;]*/g, '$1'));
        setTip({ text: element.getAttribute('data-sj-tip') ?? '', side: element.getAttribute('data-sj-tip-side') === 'right' ? 'right' : 'below', rect: element.getBoundingClientRect(), host: element.closest('dialog[open]') ?? document.body });
      }, wait);
    }
    const over = (e: PointerEvent) => { const element = find(e.target); if (element === current) return; if (element && e.pointerType !== 'touch') show(element, delay); else hide(); };
    const focus = (e: FocusEvent) => { const element = find(e.target); if (element && (e.target as Element).matches(':focus-visible')) show(element, 0); else hide(); };
    const key = (e: KeyboardEvent) => { if (e.key === 'Escape') hide(); };
    root.addEventListener('pointerover', over); root.addEventListener('pointerleave', hide); root.addEventListener('focusin', focus); root.addEventListener('focusout', hide);
    root.addEventListener('pointerdown', hide); document.addEventListener('keydown', key); window.addEventListener('scroll', hide, true); window.addEventListener('blur', hide);
    return () => {
      clearTimeout(timer);
      root.removeEventListener('pointerover', over); root.removeEventListener('pointerleave', hide); root.removeEventListener('focusin', focus); root.removeEventListener('focusout', hide);
      root.removeEventListener('pointerdown', hide); document.removeEventListener('keydown', key); window.removeEventListener('scroll', hide, true); window.removeEventListener('blur', hide);
    };
  }, []);
  // Keep a tooltip below an element inside the window.
  useLayoutEffect(() => {
    const node = tipRef.current; if (!node || !tip || tip.side !== 'below') { setOffset(0); return; }
    const box = node.getBoundingClientRect();
    setOffset(box.right > window.innerWidth - 8 ? window.innerWidth - 8 - box.right : box.left < 8 ? 8 - box.left : 0);
  }, [tip]);
  const position = tip && (tip.side === 'right'
    ? { left: tip.rect.right + 6, top: tip.rect.top + tip.rect.height / 2, transform: 'translateY(-50%)' }
    : { left: tip.rect.left + tip.rect.width / 2 + offset, top: tip.rect.bottom + 6, transform: 'translateX(-50%)' });
  return <span ref={anchor} hidden>{tip && createPortal(
    <div className="sj-theme sj-tip-layer" style={parseStyle(themeStyle)}><div ref={tipRef} role="tooltip" className="sj-tip" data-side={tip.side} style={position!}>{tip.text}</div></div>, tip.host)}</span>;
}

function parseStyle(text: string): Record<string, string> {
  const result: Record<string, string> = {};
  for (const part of text.split(';')) { const at = part.indexOf(':'); if (at > 0) { const key = part.slice(0, at).trim(); result[key.startsWith('--') ? key : key.replace(/-([a-z])/g, (_, c: string) => c.toUpperCase())] = part.slice(at + 1).trim(); } }
  return result;
}
