import { Button, Stub, stub } from '@stack-junkie/ui-web';

export default function Page() {
  return <section>
    <h1><Stub>Project name</Stub></h1>
    <Button aria-label={stub('Open project')}>Open</Button>
  </section>;
}
