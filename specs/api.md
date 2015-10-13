# Introduction

What should an API look like?

Do we want to come up with our own classes of resources and define
meaningful queries per class?  Do we want to try to mirror procfs or
cgroupfs more closely?  (I don't think so)

-- Serge

- I think we want to come up with our own classes.
- We do not want to mirror procfs or cgroupfs very closely.
- We should abstract from procfs and cgroupfs a decent amount to ensure that
  changes in either one of them do not affect us that much.
- We should come up with a list of reasonable classes. I know another potential
  contributer who I already discussed some suggestions for classes with. Maybe
  we should jump right in: hear suggestions for classes and discuss whether
  that makes sense?

-- Christian
