Copyright 2025 Ana-Cristina Calota

# Browser History Simulator

## Overview

This project implements a small command-line web browser simulator in C. It
supports multiple tabs, page navigation, backward and forward history, and
commands for printing tab and history information.

## Implementation

- A page stores an ID, URL, and description. Pages supplied in the input are
  created once and referenced by tabs and their history stacks.
- Tabs are held in a circular, doubly linked list. Each tab has its own current
  page and backward and forward stacks.
- The browser starts with a default tab on the ACS homepage. `NEW_TAB` creates
  another tab with its own default homepage.
- `PAGE` changes the active tab's page, pushes the previous page onto its
  backward history, and clears its forward history.
- `BACKWARD` and `FORWARD` move between pages by transferring page references
  between the two history stacks.
- `OPEN`, `NEXT`, and `PREV` select tabs. The default tab cannot be closed.
- `PRINT` displays the tab IDs starting from the active tab and the active
  page's description. `PRINT_HISTORY` displays a tab's forward history,
  current page, and backward history in navigation order.
- Cleanup frees pages through their owning arrays and frees history nodes,
  stacks, tabs, and list nodes through their respective owners.
