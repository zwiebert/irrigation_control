<script>
  "use strict";
  import Layout from "./layout_tailwind.svelte";
  import Wapp from "./wapp.svelte";
  import { setupI18n } from "./services/i18n";

  setupI18n();
</script>

<Layout />
<Wapp  />
