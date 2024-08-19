module Main where

import qualified MuPDF
import qualified System.Environment as Environment
import qualified System.Exit as Exit
import qualified Version

main :: IO ()
main = do
  args <- Environment.getArgs
  if any (`elem` args) ["-v", "--version"]
    then do
      putStrLn Version.version
    else do
      mupdfVersion <- MuPDF.version
      putStrLn ("mupdf: " ++ mupdfVersion)
  Exit.exitSuccess
