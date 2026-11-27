// The shape scan.mjs writes to data/projects.json.
export type Status = 'current' | 'behind' | 'ahead' | 'unknown' | 'not-using' | 'source';
export type Marker = { kind: 'package' | 'import' | 'cmake' | 'skill'; file: string; version: string | null };
export type Project = {
  id: string; name: string; where: 'local' | 'github' | 'gitlab'; path?: string; url?: string; link: string;
  links: { label: string; href: string }[];
  usesStyle: boolean; source: boolean; version: string | null; latest: string; status: Status;
  frontEnd: boolean; exempt: boolean; markers: Marker[]; unchecked?: boolean;
};
export type Source = { status: 'connected' | 'not connected' | 'off'; account?: string; repos?: number; unchecked?: number; reason?: string; roots?: number; projects?: number; errors?: number };
export type ScanData = {
  scannedAt: string; latest: string; roots: string[]; skipped?: string[];
  sources: { local: Source; github: Source; gitlab: Source };
  projects: Project[];
};
