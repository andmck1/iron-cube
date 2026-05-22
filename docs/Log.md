# Log

## Friday 22 May 2026

- Use of AI is forbidden for the project. As this work is for the renderer and
  not the actual project I think I'm okay as long as I cite it. I will pull the
  use of my agentic AI tools from the moment I start mesh shader work to be
  safe.
- First meeting with Erin today:
  - Asked about agentic AI, she shared the guidance and recommended that how
    I've used it is analogous to using another renderer.
  - Recommended that I treat the task shader as another CPU shader where other
    things can happen
  - Evaluate qualitatively and quantitatively. Note down visual issues to be
    expected when using certain methods and use heatmaps using eg NVIDIA flip.
  - Noted the scene can evolve and isn't set to what was dictated in my research
    proposal.
- Laith confirmed that as long as it's only for the renderer and I reference it
  then it's fine
- Remaining work for renderer:
  - Better logging. I need to add debug lines, pass debug as a command argument
    and output to a .log file per run.
  - Remove constructor for mesh component
  - Review pointer safety
  - Review lack of consts
  - Review graphics code
  - Review all code and de-AIify it
