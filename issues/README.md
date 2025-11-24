# Issues Tracking

This folder contains detailed documentation for bugs and issues encountered during development. Each issue gets its own markdown file.

## Purpose

- Document every bug/issue with detailed context
- Track all attempted solutions and their outcomes
- Prevent repeating failed approaches
- Maintain a historical record of debugging efforts

## File Naming Convention

Use descriptive lowercase filenames with hyphens:
- `black-screen-on-miyoo.md`
- `sdl-renderer-initialization.md`
- `network-sync-timeout.md`

## Issue File Structure

Each issue file should follow this template:

```markdown
# Issue: [Brief Description]

## Problem
Detailed description of the problem, including:
- What is happening
- When it occurs
- Error messages or symptoms
- Environment details (Miyoo device, build version, etc.)

## Expected Behavior
What should happen instead

## Attempted Solutions

### Attempt 1: [Approach Name] (STATUS)
- **Date**: YYYY-MM-DD
- **Description**: What was tried
- **Commit**: abc123 (if applicable)
- **Outcome**: What happened
- **Logs/Evidence**: Relevant logs or observations

### Attempt 2: [Approach Name] (STATUS)
- **Date**: YYYY-MM-DD
- **Description**: What was tried
- **Commit**: def456
- **Outcome**: What happened
- **Logs/Evidence**: Relevant logs or observations

## Resolution
Final solution that fixed the issue (or "TBD" if still open)

## Related Issues
Links to related issues or GitHub issues

## Lessons Learned
Key takeaways to remember for future development
```

## Status Labels

Use these status labels in attempt headers:
- **TESTING**: Waiting for validation
- **FAILED**: Approach didn't work
- **PARTIAL**: Partially solved the problem
- **SUCCESS**: Fixed the issue

## Workflow

1. When encountering a new bug, create a new issue file
2. Document the problem and expected behavior
3. Before trying a solution, check if it's been attempted before
4. After each attempt, update the issue file with the outcome
5. When resolved, document the final solution and lessons learned
6. Keep the file for historical reference (don't delete)

## Example

See issue template above for the recommended structure.
