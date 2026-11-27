import { useState, type ButtonHTMLAttributes, type HTMLAttributes, type ReactNode } from 'react';
import sprite from '../../../shared/icons.svg?raw';

// Inline the bundled, source-owned symbols. Library builds inline URL imports as
// data URLs, which SVG <use> does not support in current browsers.
const symbols = Object.fromEntries([...sprite.matchAll(/<symbol\s+id="([^"]+)"[^>]*>([\s\S]*?)<\/symbol>/g)].map(match => [match[1], match[2]]));

export type IconName = 'search' | 'chevron-left' | 'chevron-right' | 'chevron-down' | 'close' | 'menu' | 'sun' | 'moon' | 'check' | 'plus' | 'help' | 'settings' | 'grid' | 'code' | 'copy' | 'folder' | 'file' | 'alert' | 'play' | 'pause' | 'layout';
export function Icon({ name, size = 18 }: { name: IconName; size?: number }) { return <svg width={size} height={size} viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.7" strokeLinecap="round" strokeLinejoin="round" aria-hidden="true" focusable="false"><g dangerouslySetInnerHTML={{ __html: symbols[name] || '' }} /></svg>; }
export function Button({ variant = 'secondary', busy = false, children, className = '', disabled, type = 'button', ...props }: ButtonHTMLAttributes<HTMLButtonElement> & { variant?: 'primary' | 'secondary' | 'quiet' | 'danger'; busy?: boolean }) {
  return <button {...props} type={type} disabled={disabled || busy} aria-busy={busy || undefined} className={`sj-button sj-button--${variant} ${className}`}>{busy && <Spinner label="Working" />}{children}</button>;
}
export function IconButton({ icon, label, ...props }: Omit<Parameters<typeof Button>[0], 'children'> & { icon: IconName; label: string }) { return <Button {...props} aria-label={label} title={label} className={`sj-icon-button ${props.className || ''}`}><Icon name={icon} /></Button>; }
export function ToggleButton({ pressed, onPressedChange, children, ...props }: Omit<Parameters<typeof Button>[0], 'onClick'> & { pressed: boolean; onPressedChange: (pressed: boolean) => void }) { return <Button {...props} aria-pressed={pressed} onClick={() => onPressedChange(!pressed)}>{children}</Button>; }
export function Badge({ tone = 'neutral', children }: { tone?: 'neutral' | 'success' | 'warning' | 'danger'; children: ReactNode }) { return <span className={`sj-badge sj-tone-${tone}`}>{children}</span>; }
export function Alert({ tone = 'info', title, children, action }: { tone?: 'info' | 'success' | 'warning' | 'danger'; title: string; children?: ReactNode; action?: ReactNode }) { return <div className={`sj-alert sj-tone-${tone}`} role={tone === 'danger' ? 'alert' : 'status'}><Icon name={tone === 'success' ? 'check' : 'alert'} /><div><strong>{title}</strong>{children && <div>{children}</div>}</div>{action}</div>; }
export function Spinner({ label = 'Loading' }: { label?: string }) { return <span className="sj-spinner" role="status"><span className="sj-sr-only">{label}</span></span>; }
export function Progress({ value, label }: { value?: number; label: string }) { return <label className="sj-progress"><span>{label}{value !== undefined && <span className="sj-mono">{Math.round(Math.max(0, Math.min(100, value)))}%</span>}</span><progress max={100} value={value} /></label>; }
export function Skeleton({ lines = 3, label = 'Loading content' }: { lines?: number; label?: string }) { return <div className="sj-skeleton" role="status" aria-label={label}>{Array.from({ length: lines }, (_, i) => <span key={i} style={{ width: `${100 - (i % 3) * 14}%` }} />)}</div>; }
export function Toast({ children, onDismiss }: { children: ReactNode; onDismiss: () => void }) { return <div className="sj-toast" role="status"><Icon name="check" /><span>{children}</span><IconButton icon="close" label="Dismiss notification" onClick={onDismiss} variant="quiet" /></div>; }
// A Stub is development stand-in content. It renders striped, and the release guard (`stackJunkieStubGuard` in `@stack-junkie/ui-web/vite`) stops a build while any remain.
export function Stub({ block = false, children, className = '', ...props }: HTMLAttributes<HTMLElement> & { block?: boolean }) { const Tag = block ? 'div' : 'span'; return <Tag title="Stub: replace before release" {...props} data-sj-stub="" className={`sj-stub${block ? ' sj-stub--block' : ''} ${className}`}>{children}</Tag>; }
// Marks a plain value (attribute text or fake data) as a Stub, for places that cannot show stripes. The release guard finds it the same way.
export function stub<T>(value: T): T { return value; }
export function Separator() { return <hr className="sj-separator" />; }
export function Avatar({ name, size = 'regular' }: { name: string; size?: 'small' | 'regular' }) { return <span className={`sj-avatar sj-avatar--${size}`} aria-label={name} title={name}>{name.split(/\s+/).slice(0, 2).map(x => x[0]).join('')}</span>; }
export function Prose({ children, className = '', ...props }: HTMLAttributes<HTMLElement>) { return <article {...props} className={`sj-prose ${className}`}>{children}</article>; }
export function CodeBlock({ code, language = 'Text' }: { code: string; language?: string }) {
  const [copied, setCopied] = useState(false); const [error, setError] = useState('');
  async function copy() { try { await navigator.clipboard.writeText(code); setCopied(true); setError(''); } catch { setError('Copy is unavailable. Select the text to copy it.'); } }
  return <div className="sj-code"><div className="sj-code-heading"><span>{language}</span><Button variant="quiet" onClick={copy}><Icon name={copied ? 'check' : 'copy'} />{copied ? 'Copied' : 'Copy'}</Button></div><pre tabIndex={0}><code>{code}</code></pre>{error && <p role="alert">{error}</p>}</div>;
}
