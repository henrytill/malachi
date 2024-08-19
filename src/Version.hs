module Version (version) where

import qualified Data.Version as Version
import qualified Paths_malachi

version :: String
version = Version.showVersion Paths_malachi.version
