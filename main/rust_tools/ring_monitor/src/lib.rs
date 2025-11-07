

/// This file provides database functions to
/// store ring statistics as time series.
/// This is in response to Issue #383 which 
/// asks for the ability to visualize time series statistics,
/// which cannot be addressed without also having stored it in the first place.
/// 

///
/// The information we want to maintain for each ringbuffer is:
/// 
use std::time;
use serde::Serialize;
#[derive(Clone, Debug, Serialize, PartialEq)]
pub struct RingBufferStatistics {
    pub name:  String,         // name of the ringbuffer.
    pub bytes: usize,          // Total bytes observed through the ring.
    pub events: usize,         // Total PHYSICS_EVENT items observed through the ring.
    pub bytes_this_run: usize, // Total bytes through the ring since last BEGIN_RUN.
    pub events_this_run: usize, // Total PHYSICS_EVENT items since last BEGIN_RUN.
}
impl RingBufferStatistics {
    /// Create a ring buffer statistics item
    /// that has all counters zeroed:
    pub fn new(name :&str) -> RingBufferStatistics {
        RingBufferStatistics {
            name: String::from(name),
            bytes: 0, events: 0,
            bytes_this_run: 0, events_this_run: 0
        }
    }
    /// Zero the per run statistics:
    /// 
    pub fn new_run(&mut self) -> &mut RingBufferStatistics {
        self.bytes_this_run = 0;
        self.events_this_run = 0;
        self              // So chaining can be done.
    }
    /// Count some bytes:
    /// 
    pub fn count_bytes(&mut self, bytes : usize) -> &mut RingBufferStatistics {
        self.bytes = self.bytes.wrapping_add(bytes);
        self.bytes_this_run =self.bytes_this_run.wrapping_add(bytes);
        self
    }
    /// Count some events:
    ///
    pub fn count_events(&mut self, events: usize) -> &mut RingBufferStatistics {
        self.events = self.events.wrapping_add(events);
        self.events_this_run = self.events_this_run.wrapping_add(events);
        self
    }
}


///  This is the structure of statistics update messages sent
///  to the main thread by the monitor threads:
/// 
#[derive(Debug, PartialEq)]
pub struct UpdateMessage {
    pub interval : time::Duration,
    pub statistics : RingBufferStatistics,
}

impl UpdateMessage {
    pub fn new(i : time::Duration, stats : &RingBufferStatistics) -> UpdateMessage {
        UpdateMessage { interval: i, 
            statistics : stats.clone()
        }
    }
}

/// The statistics the user is interested in also include rates:
/// 
#[derive(Clone, Serialize, Debug, PartialEq)]
pub struct StatisticsAndRates {
    pub name             : String,               // yes duplicate info but for easier JSON processing.
    pub cum_statistics   : RingBufferStatistics, // Note this has the ring name.
    pub byte_rate         : f64,
    pub event_rate        : f64,
    pub byte_per_run_rate : f64,
    pub evts_per_run_rate : f64
}

impl StatisticsAndRates {
    // Create a newly initialized struc.
    // This requires a ring name.

    pub fn new(ring: &str) -> StatisticsAndRates {
        StatisticsAndRates {name: String::from(ring), cum_statistics: RingBufferStatistics::new(ring), 
             byte_rate: 0.0, event_rate: 0.0, byte_per_run_rate: 0.0, evts_per_run_rate: 0.0 }
    }
    pub fn update(&mut self, info : &UpdateMessage) -> &mut Self {
        // Let's be sure the message really was for us.
        // panic if not:

        if info.statistics.name != self.cum_statistics.name {
            let msg = 
                format!("BUGCHECK - StatiticsAndRates::update called with mismatched rings was {} should be {}", 
                info.statistics.name, self.cum_statistics.name
            );
            panic!("{}", msg);
        }


        // Need differences from last and now to get rates:

        let dt : f64 = info.interval.as_secs_f64();
        self.byte_rate = (info.statistics.bytes - self.cum_statistics.bytes) as f64 / dt;
        self.event_rate = (info.statistics.events - self.cum_statistics.events) as f64/dt;
        
        // If we started a new run and the events are < than last time, then we start from 0.
        // To compute the rate.

        if self.cum_statistics.bytes_this_run > info.statistics.bytes_this_run {
            // New run so:

            self.cum_statistics.bytes_this_run = 0;
            self.cum_statistics.events_this_run = 0;
        }
        self.byte_per_run_rate = 
            (info.statistics.bytes_this_run - self.cum_statistics.bytes_this_run) as f64 / dt;
        self.evts_per_run_rate =
            (info.statistics.events_this_run - self.cum_statistics.events_this_run) as f64 /dt;

        // Updtae the last statistics field:

        self.cum_statistics = info.statistics.clone();
        

        self                                // If we add more methods we can chain.
    }
}


pub mod database {
    use rusqlite;
    use std::collections::HashMap;
    use crate::*;
    /// This struct is provided to manipulate the database:
    /// 
    pub struct Connection {
        connection : rusqlite::Connection,
        ring_cache : HashMap<String, i64>
    }
    #[derive(Debug)]
    pub enum Error {
        CannotOpen
    }
    type DatabaseError = Result<Connection, Error>;

    impl Connection {
        // private methods:

        // Create the schema in the database,
        // panics on failure.

        fn make_schema(&self) {
            if let Err(e) = self.connection.execute(
                "CREATE TABLE IF NOT EXISTS ring_names (
                    id    INTEGER PRIMARY KEY AUTOINCREMENT,
                    name  STRING)", []
            ) {
                panic!("Unable to create the ring_names table!! {}", e);
            }
            if let Err(e) = self.connection.execute(
                "
                CREATE TABLE IF NOT EXISTS statistics (
                    id           INTEGER PRIMARY KEY AUTOINCREMENT,
                    ring_id      INTEGER,   -- FK to ring_names
                    timestamp    INTEGER,   -- unix epoch offset.
                    volume       INTEGER,   -- Total bytes
                    run_vol      INTEGER,   -- bytes this run
                    events       INTEGER,   -- total events.
                    run_events   INTEGER,   -- events this run.
                    rate         REAL,      -- bytes/per second.
                    event_rate   REAL      -- Events/second.
                )
                ", []
            ) {
                panic!("Unable to create the statistics table {}", e);
            }
        }
        // Load the ring names cache of self. panics on falure.
        fn load_ring_cache(&mut self) {
            let mut stmt = self.connection.prepare("SELECT id, name FROM ring_names")
                .expect("Could not prepare ring name query");
            let mut rows = stmt.query([]).expect("Unable to ring name query");
            while let Some(row) = rows.next().expect("Unable to fetch a ring name row") {
                let id : i64 = row.get(0).expect("Could not fetch a ring id");
                let name : String = row.get(1).expect("Could not fetch rig name");
                self.ring_cache.insert(name, id);
            }
        }
        // Get the id associated with a ring buffer, creating it in the 
        // ring_names table if needed.

        fn get_ring_id(&mut self, name : &str) -> i64 {
            if self.ring_cache.contains_key(name) {
                return *self.ring_cache.get(name).unwrap()
            } else {
                // have to create it:

                self.connection.execute("
                    INSERT INTO ring_names (name) VALUES (?)
                ", [name]).expect("Failed to make a new ring buffer name entry.");
                let result = self.connection.last_insert_rowid();
                self.ring_cache.insert(String::from(name), result);
                result
            }
        }

        // Public methods.
    
        ///
        /// Open or create a new time series database.
        /// If necessary the appropriate tables are created,
        /// The existing set of rings is read into the ring_cache.
        /// The assumption is that we are the only writer.
        pub fn new(file_name : &str) -> DatabaseError {
            let conn = rusqlite::Connection::open(file_name);
            if let Err(failed)  = conn {
                println!("Failed to open sqlite3 connection {}", failed);
                return Err(Error::CannotOpen);
            } 
            let mut result = Connection {
                connection: conn.unwrap(),
                ring_cache: HashMap::new()
            };
            result.make_schema();
            result.load_ring_cache();

            Ok(result)
        }
        pub fn log(&mut self, info : &StatisticsAndRates) {
            // If needed, add the ringbuffer to the ring table:

            let ring_id = self.get_ring_id(&info.name);
            // Get the timestamp string:
            let now = time::SystemTime::now();
            if let Ok(t) = now.duration_since(time::UNIX_EPOCH)  {
            // Now insert the statistics entry:

                if let Err(e) =  self.connection.execute(
                    "INSERT INTO statistics 
                        (ring_id, timestamp, volume, run_vol, events, run_events, rate, event_rate)
                        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
                    ",
                    rusqlite::params!(ring_id, t.as_secs(),
                        info.cum_statistics.bytes, info.cum_statistics.bytes_this_run, 
                        info.cum_statistics.events, info.cum_statistics.events_this_run,
                        info.byte_rate, info.event_rate
                    )

                ) {
                    eprintln!("Unable to make statistics entry for ring {} at {}: {}", info.name, t.as_secs(), e);
                }
            } else {
                eprintln!("Unable to convert date/tikme to unix epoch relative");
            }
        }
    }
    // module tests.
    #[cfg(test)]
    mod db_tests {
        // Tests for the database operations.
        use crate::*;
        use super::*;
        use rusqlite;
        #[test]
        fn new_1() {
            // New creates the schema:

            let db = Connection::new(":memory:");
            assert!(db.is_ok());
            let db = db.unwrap();

            // Check that the two tables have been made... at this point the
            // ring cache is empty too:

            let mut stmt = db.connection.prepare(
                "SELECT COUNT(*) FROM sqlite_master WHERE name = 'ring_names'")
                .expect("Failed to prep stmt");
            let mut rows = stmt.query([]).expect("Failed to exectute query");
            let row = rows.next().expect("There was no row result!!").expect("Could not unwrap row");
            assert_eq!(1, row.get(0).expect("Failed to get the count"));

            let mut stmt = db.connection.prepare(
                "SELECT COUNT(*) FROM sqlite_master WHERE name = 'statistics'")
                .expect("Failed to prep stmt");
            let mut rows = stmt.query([]).expect("Failed to exectute query");
            let row = rows.next().expect("There was no row result!!").expect("Could not unwrap row");
            assert_eq!(1, row.get(0).expect("Failed to get the count"));

            // There are o entries in the ring name cache.

            assert_eq!(0, db.ring_cache.len());
        }
        #[test]
        fn loadrc_1() {
            // the ring cache is loaded correctly if there is a ring buffer in the ring_names table.
            // Note the URI and query params form of the open force both connections
            // to use the same databases.

            let raw = rusqlite::Connection::open("file::memory:?cache=shared").expect("Failed to connect sqlite.");
            raw.execute("
                CREATE TABLE IF NOT EXISTS ring_names (
                        id    INTEGER PRIMARY KEY AUTOINCREMENT,
                        name  STRING)", []
            ).expect("Failed to make ring name table");
            raw.execute("
                INSERT INTO ring_names (name) VALUES('aring')
            ", []).expect("could not insert a ring.");

            // Have to sleeze this a bit to use the same memory database.
            let db = Connection::new("file::memory:?cache=shared").unwrap();

            assert_eq!(1, db.ring_cache.len());
            assert!(db.ring_cache.contains_key("aring"));
        }
        #[test]
        fn log_1() {
            // Log a statistic to the database and check the ring buffer is added to the table and cache:

            let mut db = Connection::new(":memory:").expect("Failed to make database");
            let stats = StatisticsAndRates::new("aring");
            db.log(&stats);

            // Ring was added to the cache.
            assert_eq!(1, db.ring_cache.len());

            // THere's a new ring_name entry....that's enough of a test for now.
            let mut stmt = db.connection.prepare(
                "SELECT COUNT(*) FROM ring_names WHERE name ='aring'"
            ).expect("Failed to prepare a statement");
            let mut rows = stmt.query([]).expect("Failed to query the statement");
            let row = rows.next().expect("Failed to get a row").expect("Failed to unwrap a row");
            assert_eq!(1i64, row.get(0).expect("Failed to get the count"));

        }
        #[test]
        fn log_2() {
            // Test that logging a statistic gets the right row values....we don't compare the timestamp.
            // For rates we'll use a duration of a second.

            let mut db = Connection::new(":memory:").expect("Failed to make database");
            let mut stats = StatisticsAndRates::new("aring");
            let mut totals = RingBufferStatistics::new("aring");
            totals.count_bytes(100);
            totals.count_events(10);
            let msg = UpdateMessage::new(time::Duration::from_secs(1), &totals);
            stats.update(&msg);

            // NOw we can log the stats:

            db.log(&stats);

            // THere should be one stat line:

            let mut count_stmt = db.connection.prepare(
                "SELECT COUNT(*) FROM statistics"
            ).expect("Failed to make line count stmt");
            let mut rows = count_stmt.query([]).expect("Failed to query # of lines");
            let row = rows.next().expect("Failed next").expect("Failed to unwrap row");
            assert_eq!(1i64, row.get(0).expect("Failed to get count"));

            // Get the data we can verify from the row:

            let mut stmt = db.connection.prepare("
                SELECT ring_id, volume, run_vol, events, run_events, rate, event_rate
                FROM statistics
            ").expect("Failed to prep stats get query");
            let mut rows = stmt.query([]).expect("Failed to run stats query");
            let row = rows.next().expect("Failed next on rows").expect("Failed to unwrap row");

            assert_eq!(*db.ring_cache.get("aring").unwrap(), row.get(0).unwrap());
            assert_eq!(100, row.get(1).unwrap());         // total bytes
            assert_eq!(100, row.get(2).unwrap());         // bytes this run.
            assert_eq!(10, row.get(3).unwrap());          // total events.
            assert_eq!(10, row.get(4).unwrap());          // events this run
            assert_eq!(100.0, row.get(5).unwrap());       // byte rate.

        }
    }
    #[test]
    fn log_3() {
        // Two logs to the same ring buffer result in only one
        // entry in ring_names.

        let mut db = Connection::new(":memory:").expect("Failed to make database");
        let mut stats = StatisticsAndRates::new("aring");
        let mut totals = RingBufferStatistics::new("aring");
        totals.count_bytes(100);
        totals.count_events(10);
        let msg = UpdateMessage::new(time::Duration::from_secs(1), &totals);
        stats.update(&msg);

        // NOw we can log the stats 2x

        db.log(&stats);
        db.log(&stats);

        // How may ring names do we have?

        let mut stmt = db.connection.prepare("
            SELECT COUNT(*) FROM ring_names
        ").expect("Could not prepare statement");
        let mut rows = stmt.query([]).expect("Failed to run count query");
        let row = rows.next()
            .expect("failed to iterate row")
            .expect("Failed to unwrap row");
        assert_eq!(1, row.get(0).unwrap());

    }

}

//  ----- tests -----

#[cfg(test)]
mod rbstat_tests {
    // Tests for the RingBufferStatistics implementation.
    use crate::*;

    #[test]
    fn new_1() {
        let stats = RingBufferStatistics::new("testing");
        assert_eq!(
            RingBufferStatistics {
                name: String::from("testing"), 
                bytes : 0, events: 0, bytes_this_run : 0, events_this_run: 0
            },
            stats
        );
    }
    #[test]
    fn count_bytes_1() {
        let mut stats = RingBufferStatistics::new("testing");
        stats.count_bytes(10);
        assert_eq!(
            RingBufferStatistics {
                name: String::from("testing"), 
                bytes : 10, events: 0, bytes_this_run : 10, events_this_run: 0
            },
            stats
        );
    }
    #[test]
    fn count_events_1() {
        let mut stats = RingBufferStatistics::new("testing");
        stats.count_events(5);
        assert_eq!(
             RingBufferStatistics {
                name: String::from("testing"), 
                bytes : 0, events: 5, bytes_this_run : 0, events_this_run: 5
            },
            stats
        );
    }
    #[test]
    fn new_run_1() {
        let mut stats = RingBufferStatistics::new("testing");
        stats.count_events(5)
            .count_bytes(10)
            .new_run();   // Should only clear the *this_run values:

        assert_eq!(
            RingBufferStatistics {
                name: String::from("testing"), 
                bytes : 10, events: 5, bytes_this_run : 0, events_this_run: 0
            },
            stats
        );

    }
}
#[cfg(test)]
mod updmsg_tests {
    use crate::*;
    use std::time::Duration;
    // Test the implementation of update messages:

    use crate::RingBufferStatistics;

    #[test]
    fn new_1() {
        let mut stats = RingBufferStatistics::new("test");
        stats.count_bytes(100)
            .count_events(10)
            .new_run()                 // Zero the run countes.
            .count_bytes(100)
            .count_events(10);              // More data.

        let update_time = Duration::from_secs(2);   // Yeah slow rate but meh.

        let msg = UpdateMessage::new(update_time, &stats);

        assert_eq!(
            UpdateMessage {
                interval: update_time,
                statistics: RingBufferStatistics {
                    name: String::from("test"),
                    bytes: 200, events: 20, bytes_this_run: 100, events_this_run: 10
                }
            },
            msg
        );
    }
}
#[cfg(test)]
mod sandr_tests {
    // Tests for impl StatisticsAndRates

    use crate::*;
    use std::time::Duration;

    #[test]
    fn new_1() {
        // Test proper initialization:

        let stats = StatisticsAndRates::new("test");
        assert_eq!(
            StatisticsAndRates {
                name: String::from("test"),
                cum_statistics : RingBufferStatistics {
                     name: String::from("test"), 
                     bytes: 0, events: 0, bytes_this_run: 0, events_this_run: 0 },
                byte_rate: 0.0, event_rate: 0.0,
                byte_per_run_rate: 0.0, evts_per_run_rate: 0.0
            }, stats
        );
    }
    #[test]
    fn update_1() {
        // Test update and rate computation with no new run:

        let mut stats = StatisticsAndRates::new("test");
        let interval = Duration::from_secs(1);
        let mut incr_stat = RingBufferStatistics::new("test");
        incr_stat.count_bytes(100)
            .count_events(5);
        let msg = UpdateMessage::new(interval, &incr_stat);

        stats.update(&msg);

        // Check the rates should be pretty easy with intervals like 1.0:

        assert_eq!(
            StatisticsAndRates {
                name: String::from("test"),
                cum_statistics: incr_stat.clone(),
                byte_rate: 100.0,
                event_rate: 5.0,
                byte_per_run_rate : 100.0,
                evts_per_run_rate: 5.0
            }
            ,stats
        );

    }
    #[test]
    fn update_2() {
        // Simulate a message update that indicates an new run started.

        let mut stats = StatisticsAndRates::new("test");
        let interval = Duration::from_secs(1);
        let mut incr_stat = RingBufferStatistics::new("test");
        incr_stat.count_bytes(100)
            .count_events(5);
        let msg = UpdateMessage::new(interval, &incr_stat);

        stats.update(&msg);        // Establishes baseline counts:

        incr_stat.new_run()
            .count_bytes(50)
            .count_events(2);             // Should make stats thing this is a new run.
        let msg = UpdateMessage::new(interval, &incr_stat);
        stats.update(&msg);
        
        assert_eq!(
            StatisticsAndRates {
                name: String::from("test"),
                cum_statistics : RingBufferStatistics {
                    name : String::from("test"), 
                    bytes: 150, events: 7, bytes_this_run: 50, events_this_run: 2

                },
                byte_rate: 50.0, event_rate: 2.0, byte_per_run_rate : 50.0, evts_per_run_rate: 2.0
            },
            stats
        )

    }
    #[test]
    fn update_3() {
        // Two updates that change rate but no runs:

        let mut stats = StatisticsAndRates::new("test");
        let interval = Duration::from_secs(1);
        let mut incr_stat = RingBufferStatistics::new("test");
        incr_stat.count_bytes(100)
            .count_events(5);
        let msg = UpdateMessage::new(interval, &incr_stat);

        stats.update(&msg);        // Establishes baseline counts:

        incr_stat.count_bytes(50)
            .count_events(2);             // Should make stats thing this is a new run.
        let msg = UpdateMessage::new(interval, &incr_stat);
        stats.update(&msg);
        
        assert_eq!(
            StatisticsAndRates {
                name: String::from("test"),
                cum_statistics : RingBufferStatistics {
                    name : String::from("test"), 
                    bytes: 150, events: 7, bytes_this_run: 150, events_this_run: 7

                },
                byte_rate: 50.0, event_rate: 2.0, byte_per_run_rate : 50.0, evts_per_run_rate: 2.0
            },
            stats
        )

    }
}

