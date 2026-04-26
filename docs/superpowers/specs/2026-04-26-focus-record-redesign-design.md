# Focus Record Redesign Design

Date: 2026-04-26

## Context

The current pomodoro and stopwatch pages write focus records directly into diary text with multiple incompatible line formats. Pomodoro, stopwatch, diary display, and statistics each parse those lines separately, so small wording or encoding differences can break synchronization.

Known issues this design addresses:

- The pomodoro action "end early" is ambiguous when a plan contains multiple rounds. It is unclear whether it ends only the current round or the whole plan.
- Stopwatch "stop and record" writes a diary entry but leaves the elapsed timer visible and reusable, which makes accidental duplicate or accumulated records likely.
- Pomodoro-to-diary synchronization is fragile because daily totals and detail lines are recalculated from free-form diary text.
- Diary focus records need better granularity: each record should keep task, type, round, status, start/end time, planned duration, and actual duration where applicable.

## Goals

- Make timer actions explicit and unambiguous.
- Record both a readable daily summary and detailed focus events in the diary.
- Keep user diary text separate from generated focus records.
- Make statistics and diary rendering use one shared parsing path.
- Preserve existing diary files as much as possible and continue reading old focus entries.

## Non-Goals

- Add cloud synchronization or conflict resolution.
- Move focus records to a database.
- Redesign the full diary editor UI beyond the generated focus block.
- Change accounting, OCR, image, or login flows.

## Chosen Approach

Use a structured focus record line stored inside the existing daily diary text file. The app will generate and parse these lines consistently, then render them as a human-readable summary and detail block in the diary page.

This keeps storage simple while removing the most fragile free-form parsing from timers and statistics.

## Alternatives Considered

1. Continue writing free-form diary text.
   - Lowest code change.
   - Still fragile because every consumer must parse human text.
   - Not recommended.

2. Store structured focus records in the diary file.
   - Moderate code change.
   - Keeps existing file model.
   - Supports both summary and detailed records.
   - Recommended.

3. Store focus records in separate data files.
   - Cleanest data boundary.
   - Larger migration and sync surface.
   - Too heavy for the current scope.

## Focus Record Format

New generated records will be single-line entries with stable keys:

```text
[FocusRecord type=pomodoro task="Write paper" round=2/4 status=completed start=09:00 end=09:25 plannedMin=25 actualSec=1500 actualMin=25]
[FocusRecord type=pomodoro task="Write paper" round=3/4 status=ended-current start=09:35 end=09:47 plannedMin=25 actualSec=720 actualMin=12]
[FocusRecord type=stopwatch task="Reading" status=recorded start=14:10 end=14:42 actualSec=1920 actualMin=32]
```

Rules:

- `type` is `pomodoro` or `stopwatch`.
- `task` is quoted and escaped so spaces and Chinese text remain safe.
- `round` exists only for pomodoro records and uses `current/total`.
- `status` values are:
  - `completed`: a pomodoro focus round reached zero naturally.
  - `ended-current`: the user ended the current pomodoro focus round and recorded actual time.
  - `recorded`: a stopwatch session was stopped and recorded.
- `start` and `end` use local `HH:mm`.
- `plannedMin` exists for pomodoro records.
- `actualSec` is the source of truth for duration.
- `actualMin` is stored for readability and uses rounded-up minutes.

## Diary Rendering

Diary loading will split content into:

- Generated focus records.
- Generated legacy focus lines.
- Image lines.
- User diary body.

The editor will render a generated block above the user's diary body:

```text
[今日专注汇总：共 57 分钟；番茄钟 2 次；正向计时 1 次]

[专注明细]
09:00-09:25 番茄钟 1/4 完成：写论文，25 分钟
09:35-09:47 番茄钟 2/4 提前结束当前轮：写论文，12 分钟
14:10-14:42 正向计时：阅读，32 分钟

----------------------------------
日记正文...
```

Saving the diary should avoid duplicating generated summary text. If the generated block is present in the editor, save should strip the rendered summary and details, preserve the canonical focus record lines that were loaded from the file, and persist those canonical `[FocusRecord ...]` lines plus the user's diary body and image lines.

The generated focus block is display-only. Editing the rendered summary or detail text does not modify canonical focus records in this scope.

Existing old lines such as `[Pomodoro]`, `[Stopwatch]`, and Chinese `[专注时长：...]` entries should remain readable. They may be included in summaries, but newly written records should always use `[FocusRecord ...]`.

## Pomodoro Interaction

Replace the ambiguous single "提前结束" behavior with explicit operations:

- `结束当前专注并记录`
  - Available during focus.
  - Records actual focus time if greater than zero.
  - Moves to rest if there are remaining rounds, or completes the plan if this was the last round.

- `跳过当前休息`
  - Available during rest.
  - Does not create a focus record.
  - Moves to the next focus round or completes the plan if no rounds remain.

- `结束整个计划`
  - Available while a pomodoro plan is running or paused.
  - Stops the timer and leaves already recorded rounds intact.
  - Does not record the unfinished current rest period.
  - If invoked during a focus period with elapsed focus time, prompt the user to choose whether to record that partial focus time or discard it.

- `重置`
  - Stops the timer, clears current progress, and returns the display to the configured focus duration.
  - Does not write any record.

The page should also display current phase and round, for example `专注 2/4` or `休息 2/4`, so users can tell exactly what will happen.

## Stopwatch Interaction

`结束并记录` will:

- Stop the timer.
- Write one `[FocusRecord type=stopwatch ...]` entry if elapsed seconds are greater than zero.
- Reset elapsed time to zero and update the display to `00:00:00`.
- Keep the remark text after recording so repeated sessions for the same task are convenient.

`重置` will:

- Stop the timer.
- Reset elapsed time to zero.
- Not write a record.

If the user clicks `结束并记录` at zero seconds, the app should show a short message and avoid writing an empty record.

## Shared Focus Record Service

Add a focused service layer, tentatively `FocusRecordService`, with these responsibilities:

- Build pomodoro and stopwatch record lines.
- Parse `[FocusRecord ...]` lines into typed records.
- Parse legacy focus lines for backward-compatible summaries.
- Append new records to the correct diary date.
- Produce daily summary data and formatted detail lines.

Timer pages should call this service instead of manually scanning and rewriting diary files.

Diary and statistics pages should also use this service so totals are derived from one parser.

## Data Flow

1. User starts a pomodoro or stopwatch session.
2. Page records start timestamp and runtime state.
3. Completion or explicit record action builds a structured focus record.
4. Service appends the record to today's diary file.
5. Diary page loads the file, separates generated records from user text, and renders summary plus details.
6. Statistics service summarizes records through the same parser.

## Error Handling

- If the user is not logged in, timer record actions should not write data and should leave the timer state predictable.
- Malformed `[FocusRecord ...]` lines should be ignored for statistics but preserved in the diary text.
- Empty task names should render as `未命名任务`.
- Partial pomodoro focus durations under one minute should still be recorded using seconds internally and one rounded-up display minute.

## Testing Plan

Use test-first changes during implementation:

- Parser test: reads valid pomodoro and stopwatch records with quoted Chinese task text.
- Parser test: ignores malformed records without crashing.
- Summary test: totals pomodoro and stopwatch records from one day.
- Legacy test: old Chinese focus lines and old `[Pomodoro]` / `[Stopwatch]` lines still contribute to summaries where possible.
- Stopwatch behavior test: stop-and-record resets elapsed time and display state after writing a record.
- Pomodoro behavior test: ending current focus records only the current round and does not imply ending the whole plan.
- Diary rendering test: generated summary is not duplicated after save/load.

## Migration Strategy

No bulk migration is required. Existing diary files remain on disk unchanged.

Implementation should be backward compatible:

- New records are written as `[FocusRecord ...]`.
- Old records are read for summary and display when recognized.
- Once a user saves a diary page, generated summary blocks should not be persisted as canonical data.

## Acceptance Criteria

- Pomodoro controls make it clear whether the user is ending the current focus round, skipping rest, ending the whole plan, or resetting.
- Completing or ending a pomodoro focus round writes one detailed record with task, round, status, start/end, planned duration, and actual duration.
- Stopwatch `结束并记录` writes one detailed record and resets the timer display to zero.
- Diary shows a generated daily focus summary plus detailed records above user diary text.
- Saving and reloading a diary does not duplicate summary or detail blocks.
- Statistics and diary totals agree for the same date range.
- Existing diary files with older focus lines remain readable.
