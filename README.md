# Schedule

This is a modified job scheduler of `PosixEnv::Schedule` in [leveldb](https://github.com/google/leveldb).

`PosixEnv::Schedule` is a very simple scheduler based on FIFO queue. However, it doesn't provide interfaces like stop, pause. And also, time-related schedule is not supported. `Schedule` is a upgrade version of `PosixEnv::Schedule`, which provides functions: 

1. timer tasks schedule.
2. pause and continue to schedule.
3. stop scheduling.
